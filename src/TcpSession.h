/*
	This file is part of tcplog.

	Copyright (c) 2015, Packetsled. All rights reserved.

	tcplog is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	tcplog is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with tcplog.  If not, see <http://www.gnu.org/licenses/>.

 */

#ifndef BRO_PLUGIN_PS_TCPLOG_TCPSESSION_H
#define BRO_PLUGIN_PS_TCPLOG_TCPSESSION_H
#include <string>
#include "tcplog.bif.h"
#include "threading/MsgThread.h"
#include <DebugLogger.h>

#include <ostream>

#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/lockfree/queue.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/error.hpp>
#include <assert.h>

#include <iostream>

// 16K slots, each 16K in size
#define QUEUE_CAPACITY_MED (1024*16)
#define WORK_ITEM_BUFFER_SIZE_MED (1024*16)

// 16 slots, each 1M in size
#define QUEUE_CAPACITY_LARGE (16)
#define WORK_ITEM_BUFFER_SIZE_LARGE (1024*1024)

// Timeout in milliseconds (10s)
const unsigned int timeout_ms = 10000;

#define HEADER_SIZE (sizeof(uint32_t))

namespace plugin {
namespace PS_tcplog {
using boost::asio::ip::tcp;

// in order to use lockfree queues we cannot allocate the queue items, thus
// the queue items must have predefined sizes
//
// so, we create two different queues, one for the "average" size objects
// and another for "large" objects
typedef struct workitem_struct_med {
	char buf[WORK_ITEM_BUFFER_SIZE_MED];
	uint32_t len;
	workitem_struct_med() {
	}
	workitem_struct_med(const std::string attrs, const ODesc &buffer) {
		len = buffer.Len() + attrs.length();
		assert(len + HEADER_SIZE < WORK_ITEM_BUFFER_SIZE_MED);
		const u_char* bytes = buffer.Bytes();
		uint32_t nlen = htonl(len);
		memcpy(buf, &nlen, HEADER_SIZE);
		memcpy(buf + HEADER_SIZE, bytes, 1);
		memcpy(buf + HEADER_SIZE + 1, attrs.c_str(), attrs.length());
		memcpy(buf + HEADER_SIZE + 1 + attrs.length(), bytes + 1,
				buffer.Len() - 1);
		len += HEADER_SIZE;
	}
} workitem_med;

typedef struct workitem_struct_large {
	char buf[WORK_ITEM_BUFFER_SIZE_LARGE];
	uint32_t len;
	workitem_struct_large() {
	}
	workitem_struct_large(const std::string attrs, const ODesc &buffer) {
		len = buffer.Len() + attrs.length();
		assert(len + HEADER_SIZE < WORK_ITEM_BUFFER_SIZE_LARGE);
		const u_char* bytes = buffer.Bytes();
		uint32_t nlen = htonl(len);
		memcpy(buf, &nlen, HEADER_SIZE);
		memcpy(buf + HEADER_SIZE, bytes, 1);
		memcpy(buf + HEADER_SIZE + 1, attrs.c_str(), attrs.length());
		memcpy(buf + HEADER_SIZE + 1 + attrs.length(), bytes + 1,
				buffer.Len() - 1);
		len += HEADER_SIZE;
	}
} workitem_large;

typedef boost::lockfree::queue<workitem_med,
		boost::lockfree::capacity<QUEUE_CAPACITY_MED>,
		boost::lockfree::fixed_sized<true> > workqueue_med;

typedef boost::lockfree::queue<workitem_large,
		boost::lockfree::capacity<QUEUE_CAPACITY_LARGE>,
		boost::lockfree::fixed_sized<true> > workqueue_large;

class TcpSession: public boost::enable_shared_from_this<TcpSession> {
public:
	TcpSession() :
			io_service_(), work_(io_service_), socket_(io_service_),
			connection_active_(false), session_active_(false), drain_and_done_(false) {

		struct timeval tv;
		tv.tv_sec  = timeout_ms / 1000;
		tv.tv_usec = timeout_ms % 1000;

		setsockopt(socket_.native(), SOL_SOCKET, SO_RCVTIMEO,  &tv, sizeof(tv));
		setsockopt(socket_.native(), SOL_SOCKET, SO_SNDTIMEO,  &tv, sizeof(tv));
		setsockopt(socket_.native(), SOL_SOCKET, SO_KEEPALIVE, &tv, sizeof(tv));
	}

	~TcpSession() {
		Kill();
	}

	bool write(const std::string attrs, const ODesc &buffer) {
		bool ret = true;

		if (connection_active_) {
			int t_size = (buffer.Len() + HEADER_SIZE + attrs.length());
			if (t_size < WORK_ITEM_BUFFER_SIZE_MED) {
				workq_med.push(workitem_struct_med(attrs, buffer));
			} else if (t_size < WORK_ITEM_BUFFER_SIZE_LARGE) {
				workq_large.push(workitem_struct_large(attrs, buffer));
			} else {
				ret = false;
			}
		}

		return ret;
	}
	void Start() {
		std::cout << "{\"opcode\":\"PS_tcplog\", \"msg\":\"Start\"}" << std::endl;
		run_thread_ = boost::thread(boost::bind(&TcpSession::Run, shared_from_this()));
	}
	void Restart() {
		std::cout << "{\"opcode\":\"PS_tcplog\", \"msg\":\"Restart\"}" << std::endl;
		session_active_ = true;
		connection_active_ = true;
		CreateClientThread();
	}
	void Stop(bool io_service_stop=true) {
		std::cout << "{\"opcode\":\"PS_tcplog\", \"msg\":\"Stop\", \"io_service_stop\": "<< io_service_stop<<"}" << std::endl;
		connection_active_ = false;
		try {
			boost::system::error_code ec;
			socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
			socket_.close();
		} catch (...) {
			std::cout << "{\"opcode\":\"PS_tcplog\", \"msg\":\"StartException\"}" << std::endl;
		}

		if(io_service_stop) {
			io_service_.stop();
			io_service_.reset();
		}
	}
	void Kill() {
		std::cout << "{\"opcode\":\"PS_tcplog\", \"msg\":\"Kill\"}" << std::endl;
		session_active_ = false;
		Stop();
	}
	void Drain() {
		std::cout << "{\"opcode\":\"PS_tcplog\", \"msg\":\"Drain\"}" << std::endl;
		drain_and_done_ = true;
		session_active_ = false;
		tg.join_all();
		Kill();

	}
protected:
private:
	void CreateClientThread() {
		tg.add_thread(new boost::thread(boost::bind(&TcpSession::clientThread, shared_from_this())));
	}
	void Run() {
		std::cout << "{\"opcode\":\"PS_tcplog\", \"msg\":\"Run\"}" << std::endl;
		session_active_ = true;
		while (session_active_) {
			connection_active_ = true;
			{
				CreateClientThread();
				tg.add_thread(
						new boost::thread(boost::bind(&boost::asio::io_service::run,
											&io_service_)));
				tg.join_all();
			}
		}
	}
	void wait() {
		std::cout << "{\"opcode\":\"PS_tcplog\", \"msg\":\"Wait\"}" << std::endl;
		boost::this_thread::sleep(boost::posix_time::seconds(10));
	}
	void exError() {
		std::cout << "{\"opcode\":\"PS_tcplog\", \"msg\":\"exError\"}" << std::endl;
		Stop();
		wait();
		Restart();
	}
	void exError(std::exception& e) {
		std::cout << "{\"opcode\":\"PS_tcplog\", \"msg\":\"exError\", \"what\":\""<< e.what()<<"\"}" << std::endl;
		Stop();
		wait();
		Restart();
	}
	void asioError(const boost::system::error_code & error) {
		std::cout << "{\"opcode\":\"PS_tcplog\", \"msg\":\"asioError\", \"error\":\""<< error.value() <<"\"}" << std::endl;
		switch(error.value()) {
			case boost::system::errc::connection_refused:
			case boost::asio::error::connection_aborted:
			case boost::asio::error::connection_reset:
			case boost::asio::error::operation_aborted:
			case boost::system::errc::broken_pipe:
			case boost::asio::error::eof: {
				wait();
				Restart();
			}
				break;
			case boost::system::errc::already_connected: {
				Stop(false);
				wait();
				Restart();
			}
				break;
			default: {
				Stop();
				wait();
			}
				break;
		}
	}
		void writeSessionHeader() {
    		boost::system::error_code error;
    		ODesc sessionHeader;
    		std::stringstream ss;
    		ss << "{\"probe\": " << BifConst::PS_tcplog::probeid->CheckString() << ", \"envid\": " << BifConst::PS_tcplog::envid->CheckString() << ", \"log\": \"ps_tcplog_session\"}";
    		sessionHeader.AddRaw(ss.str());
    		workitem_struct_med sessionHeaderMessage("", sessionHeader);
    		boost::asio::write(socket_,	boost::asio::buffer(sessionHeaderMessage.buf, sessionHeaderMessage.len), error);
    		if (error) {
    			asioError(error);
    		}
    	}
	void clientThread() {
		boost::system::error_code error;
		workitem_med val;
		workitem_large val2;
		try {
			string tcphost = string((const char *) BifConst::PS_tcplog::tcphost->Bytes(),
									BifConst::PS_tcplog::tcphost->Len());
			int tcpport = atoi(BifConst::PS_tcplog::tcpport->CheckString());
			socket_.set_option(boost::asio::socket_base::reuse_address(true), error);
			socket_.set_option(boost::asio::socket_base::keep_alive(true), error);

			socket_.connect(boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string(tcphost), tcpport), error);

			//std::cout << "PS_tcplog - Successfully connected." << std::endl;

			if (error) {
				connection_active_ = false;
				session_active_ = false;
				asioError(error);
			} else {
				writeSessionHeader();

				while (connection_active_) {

					bool noWork = true;

					if (workq_med.pop(val)) {
						noWork = false;
						boost::asio::write(socket_,	boost::asio::buffer(val.buf, val.len), error);
						if (error) {
							asioError(error);
						}
					}
					if (workq_large.pop(val2)) {
						noWork = false;
						boost::asio::write(socket_,	boost::asio::buffer(val2.buf, val2.len), error);
						if (error) {
							asioError(error);
						}
					}
					if (noWork) {
						if(drain_and_done_) {
							io_service_.stop();
							break;
						} else {
							boost::this_thread::sleep(boost::posix_time::milliseconds(100));
						}
					}
				}
			}
		} catch (std::exception& e) {
			exError(e);
		} catch (...) {
			exError();
		}
	}

	boost::thread_group tg;
	workqueue_med workq_med;
	workqueue_large workq_large;
	boost::asio::io_service io_service_;
	boost::asio::io_service::work work_;
	tcp::socket socket_;
	bool connection_active_;
	bool session_active_;
	bool drain_and_done_;
	boost::thread run_thread_;
};
}
}
#endif
