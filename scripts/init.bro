###	This file is part of tcplog.

###	Copyright (c) 2015, Packetsled. All rights reserved.

###	tcplog is free software: you can redistribute it and/or modify
###	it under the terms of the GNU General Public License as published by
###	the Free Software Foundation, either version 3 of the License, or
###	(at your option) any later version.

###	tcplog is distributed in the hope that it will be useful,
###	but WITHOUT ANY WARRANTY; without even the implied warranty of
###	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
###	GNU General Public License for more details.

###	You should have received a copy of the GNU General Public License
###	along with tcplog.  If not, see <http://www.gnu.org/licenses/>.


@load base/frameworks/cluster
@load policy/misc/loaded-scripts
@load base/frameworks/communication

module PS_tcplog;

export {
        ## IP Address for PS::tcplog
        const tcphost = "127.0.0.1" &redef;
        ## Port number for PS::tcplog
        const tcpport = 1514 &redef;
        ## Sensor identifier
        const probeid = 0 &redef;
        ## Environment identifier
        const envid = 0 &redef;
        ## Enabled flag
        const enabled = T &redef;
        ## Write logfiles to disk in addition to streaming them
        const logfiles = F &redef;
        ## Log file IDs to exclude
        const excluded_log_ids: set[Log::ID] &redef;
        ## Log file IDs to write to disk
        const force_to_disk_log_ids: set[Log::ID] &redef;

        ## Flag for dependent scripts to check that tcplog plugin is loaded
        global TCPLOG_PLUGIN: bool = T &redef;
}

redef PS_tcplog::enabled = ( Cluster::local_node_type() == Cluster::LOGGER || Cluster::local_node_type() == Cluster::MANAGER || Cluster::local_node_type() == Cluster::NONE );
redef PS_tcplog::tcphost = getenv("EXPORT_ADDR");
redef PS_tcplog::tcpport = to_count(getenv("EXPORT_PORT"));
redef PS_tcplog::envid = to_count(getenv("PROBE_ENV"));
redef PS_tcplog::probeid = to_count(getenv("PROBE_ID"));

event bro_init() &priority=-5
        {
        Log::disable_stream(Communication::LOG);
        for ( sid in Log::active_streams )
                {
                if( sid !in excluded_log_ids ) {
                        #print sid;
                        local filt: Log::Filter = [$name = "ps-tcplog-filter",
                                                     $writer = Log::WRITER_TCPLOG];
                        Log::add_filter(sid, filt);
                        if(!logfiles && (sid !in force_to_disk_log_ids))
                                {
                                Log::remove_filter(sid, "default");
                                }
                        }
                }
        }
