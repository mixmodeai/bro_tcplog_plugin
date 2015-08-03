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

#ifndef BRO_PLUGIN_PS_TCPLOG
#define BRO_PLUGIN_PS_TCPLOG

#include "logging/WriterBackend.h"
#include <plugin/Plugin.h>
#include <DebugLogger.h>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include "TcpLog.h"


using namespace plugin::PS_tcplog;
using namespace logging;

namespace plugin {
namespace PS_tcplog {

class Plugin: public ::plugin::Plugin {
public:
	Plugin();
	~Plugin();
	boost::shared_ptr<TcpSession> GetSession();
	static WriterBackend* Instantiate(WriterFrontend* frontend);
	virtual void InitPostScript();
	virtual void Done();
protected:
	// Overridden from plugin::Plugin.
	virtual plugin::Configuration Configure();
private:
	boost::shared_ptr<TcpSession> session;
};

extern Plugin plugin;

}
}

#endif
