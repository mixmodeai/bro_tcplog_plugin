cmake_minimum_required(VERSION 2.8)

project(Plugin)

include(BroPlugin)

set(BOOST_ROOT ../../common/boost/install/)
set(Boost_NO_SYSTEM_PATHS ON)
set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost 1.55.0 COMPONENTS thread system REQUIRED)

if (Boost_FOUND)
        include_directories(BEFORE ${Boost_INCLUDE_DIR})
        bro_plugin_begin(PS tcplog)
        bro_plugin_cc(src/Plugin.cc src/TcpLog.cc)
        bro_plugin_bif(src/tcplog.bif)
        bro_plugin_link_library (${Boost_LIBRARIES})
        bro_plugin_end()
else ()
        message(FATAL_ERROR "Boost not found.")
endif ()
