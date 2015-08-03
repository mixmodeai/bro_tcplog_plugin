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

#include "TcpLog.h"
#include "threading/SerialTypes.h"
#include <iostream>
#include <fstream>
using namespace std;
using namespace logging::writer;
using namespace plugin::PS_tcplog;

TcpLog::TcpLog(WriterFrontend* frontend,
		boost::shared_ptr</**/::plugin::PS_tcplog::TcpSession> sess) :
		WriterBackend(frontend), session(sess) {
	json = new threading::formatter::JSON(this,
			threading::formatter::JSON::TS_EPOCH);
}

TcpLog::~TcpLog() {
	delete json;
}

bool TcpLog::DoInit(const WriterInfo& info, int num_fields,
		const threading::Field* const * fields) {
	path = Fmt("\"probe\": %d, \"envid\": %d, \"log\": \"%s\",",
			BifConst::PS_tcplog::probeid, BifConst::PS_tcplog::envid,
			info.path);
	ofstream schema_file;
	schema_file.open(Fmt("%s.schema", info.path), ios::trunc);
	schema_file << info.path << endl;
	for (int i = 0; i < num_fields; ++i) {
		const threading::Field* field = fields[i];
		schema_file << field->name << ": "
				<< GetTableType(field->type, field->subtype) << endl;
	}
	schema_file.close();
	return true;
}

bool TcpLog::DoFlush(double network_time) {
	return true;
}

bool TcpLog::DoFinish(double network_time) {
	return true;
}

bool TcpLog::DoWrite(int num_fields, const threading::Field* const * fields,
		threading::Value** vals) {
	if (session) {
		ODesc buffer;
		json->Describe(&buffer, num_fields, fields, vals);

		if(session->write(path, buffer) == false) {
			int t_size = (buffer.Len() + HEADER_SIZE + path.length());
			Error(Fmt("TcpLog::DoWrite...session->write: Dropping data - Size: %d", t_size));
		}
	}
	return true;
}

bool TcpLog::DoRotate(const char* rotated_path, double open, double close,
		bool terminating) {
	FinishedRotation();
	return true;
}

bool TcpLog::DoSetBuf(bool enabled) {
	return true;
}

bool TcpLog::DoHeartbeat(double network_time, double current_time) {
	return true;
}

string TcpLog::GetTableType(int arg_type, int arg_subtype) {
	string type;

	switch (arg_type) {
	case TYPE_BOOL:
		type = "boolean";
		break;

	case TYPE_INT:
	case TYPE_COUNT:
	case TYPE_COUNTER:
	case TYPE_PORT:
		type = "number";
		break;

	case TYPE_SUBNET:
	case TYPE_ADDR:
		type = "address";
		break;

	case TYPE_TIME:
		type = "time";
		break;
	case TYPE_INTERVAL:
	case TYPE_DOUBLE:
		type = "double precision";
		break;

	case TYPE_ENUM:
	case TYPE_STRING:
	case TYPE_FILE:
	case TYPE_FUNC:
		type = "text";
		break;

	case TYPE_TABLE:
	case TYPE_VECTOR:
		type = "text";
		break;

	default:
		type = Fmt("%d", arg_type);
	}

	return type;
}
