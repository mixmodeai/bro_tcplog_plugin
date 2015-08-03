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

#include "Plugin.h"

namespace plugin {
namespace PS_tcplog {
Plugin plugin;
}
}

Plugin::Plugin() :
		session() {
}
Plugin::~Plugin() {
	session.reset();
}
void Plugin::InitPostScript() {
	::plugin::Plugin::InitPostScript();
	if (BifConst::PS_tcplog::enabled) {
		session.reset(new TcpSession());
		session->Start();
	}
}
void Plugin::Done() {
	if (session) {
		session->Kill();
	}
	::plugin::Plugin::Done();
}
WriterBackend* Plugin::Instantiate(WriterFrontend* frontend) {
	return new ::logging::writer::TcpLog(frontend, plugin.GetSession());
}

boost::shared_ptr<TcpSession> Plugin::GetSession() {
	return session;
}

plugin::Configuration Plugin::Configure() {
	AddComponent(new ::logging::Component("TcpLog", Plugin::Instantiate));

	plugin::Configuration config;
	config.name = "PS::tcplog";
	config.description = "tcp log stream";
	config.version.major = 1;
	config.version.minor = 0;
	return config;
}
