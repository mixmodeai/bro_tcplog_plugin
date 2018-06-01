![PacketSled Logo](https://packetsled.com/wp-content/themes/freshbiz/img/packetsled-logo.png)
PS::tcplog - a Bro plugin to stream logs over tcp
==================================================================

Introduction
------------------------------------------------------------------
tcplog is a Bro plugin that will consolidate all logs into a single binary tcp stream (socket) of JSON messages. This is
useful as a high performance replacement for other log monitoring tools as it requires no dependencies be installed and
it runs at native compiled C++ speed. Additionally, the consolidation of logs into a single stream serves the use case
of ingesting data from Bro sensors into cloud models.


Prerequisites
------------------------------------------------------------------
    Bro 2.4
    boost 1.55 or greater

Compilation and Installation
------------------------------------------------------------------
To compile the plugin, you will need to point to an existing bro source tree.

    cd <path to>/tcplog
    ./configure --bro-dist=$PATH_TO_SRC_TREE_FOR/bro
    make -j10
    make install

Configuration
------------------------------------------------------------------
in local.bro:

    redef PS_tcplog::tcphost = string: ip address of host socket to connect to;
    redef PS_tcplog::tcpport = string: port to connect to;
    redef PS_tcplog::probeid = string: unique id of this sensor;
    redef PS_tcplog::envid = string: environment id of this sensor;
    redef PS_tcplog::enabled = boolean: (T/F) whether or not plugin is enabled: default T;
    redef PS_tcplog::logfiles = boolean: (T/F) whether or not to also write logfiles to filesystem: default F;
    redef PS_tcplog::excluded_log_ids += set[Log::ID] list of log id's to exclude from tcp streaming behavior;

Verification
------------------------------------------------------------------
[bash]# /usr/local/bro/bin/bro -N | grep tcplog
PS::tcplog - tcp log stream (dynamic, version 1.0)

After starting bro, a tcp session will be initiated to [PS_tcplog::tcphost]:[PS_tcplog::tcpport] and binary data will
start to arrive in the following format:

    [unsigned 4 byte network ordered payload length N]
    [N bytes of JSON text]
    [unsigned 4 byte network ordered payload length N]
    [N bytes of JSON text]
    [unsigned 4 byte network ordered payload length N]
    [N bytes of JSON text]
    [unsigned 4 byte network ordered payload length N]
    [N bytes of JSON text]
    [unsigned 4 byte network ordered payload length N]
    [N bytes of JSON text]
    ...

When the connection is dropped or terminated the plugin will wait 10 seconds before attempting to reconnect, ad nauseum.


How it works
------------------------------------------------------------------
The consolidation of the individual logs in a multi-threaded environment is performed by using a boost lockfree queue.
Multiple writers (log streams in bro) place messages into the queue. A single thread drains the queue to a socket.

License
------------------------------------------------------------------
GPL

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
