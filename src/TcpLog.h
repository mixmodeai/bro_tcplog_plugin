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

#ifndef LOGGING_WRITER_TCPLOG_H
#define LOGGING_WRITER_TCPLOG_H

#include "logging/WriterBackend.h"
#include "threading/formatters/JSON.h"
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include "TcpSession.h"

namespace logging {
namespace writer {
class msg_profile {
public:
	msg_profile() : range_max(0), range_cnt(0), range_avg(0), range_lmt(0) {}
	size_t range_max;
	size_t range_cnt;
	size_t range_avg;
	size_t range_lmt;
};
class TcpLog: public WriterBackend {
public:
	TcpLog(WriterFrontend* frontend,
			boost::shared_ptr</**/::plugin::PS_tcplog::TcpSession> sess);
	~TcpLog();

protected:
	virtual bool DoInit(const WriterInfo& info, int num_fields,
			const threading::Field* const * fields);

	virtual bool DoWrite(int num_fields, const threading::Field* const * fields,
			threading::Value** vals);
	virtual bool DoSetBuf(bool enabled);
	virtual bool DoRotate(const char* rotated_path, double open, double close,
			bool terminating);
	virtual bool DoFlush(double network_time);
	virtual bool DoFinish(double network_time);
	virtual bool DoHeartbeat(double network_time, double current_time);

private:
	string GetTableType(int, int);
	threading::formatter::JSON *json;
	boost::shared_ptr</**/::plugin::PS_tcplog::TcpSession> session;
	std::string path;
	bool profile_tcplog;
	int bytesSent, bytesDropped;
	enum { NUM_RANGE = 64 };
	msg_profile range[NUM_RANGE];
	size_t num_profile;
	size_t secondCountDown;
};

}
}

#endif
