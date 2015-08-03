##	This file is part of tcplog.

##	Copyright (c) 2015, Packetsled. All rights reserved.

##	tcplog is free software: you can redistribute it and/or modify
##	it under the terms of the GNU General Public License as published by
##	the Free Software Foundation, either version 3 of the License, or
##	(at your option) any later version.

##	tcplog is distributed in the hope that it will be useful,
##	but WITHOUT ANY WARRANTY; without even the implied warranty of
##	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
##	GNU General Public License for more details.

##	You should have received a copy of the GNU General Public License
##	along with tcplog.  If not, see <http://www.gnu.org/licenses/>.


@load base/frameworks/cluster
@load policy/misc/loaded-scripts

module PS_tcplog;

export {
        const tcphost = "127.0.0.1" &redef;
        const tcpport = 1514 &redef;
        const probeid = 0 &redef;
        const envid = 0 &redef;
        const enabled = T &redef;
        const logfiles = F &redef;
        const excluded_log_ids: set[Log::ID] &redef;
}

redef PS_tcplog::enabled = ( Cluster::local_node_type() == Cluster::MANAGER || Cluster::local_node_type() == Cluster::NONE );

event bro_init() &priority=-5
        {
        for ( sid in Log::active_streams )
                {
                if( sid !in excluded_log_ids ) {
                        print sid;
                        local filt: Log::Filter = [$name = "ps-tcplog-filter",
                                                     $writer = Log::WRITER_TCPLOG];
                        Log::add_filter(sid, filt);
                        if(!logfiles)
                                {
                                Log::remove_filter(sid, "default");
                                }
                        }
                }
        }