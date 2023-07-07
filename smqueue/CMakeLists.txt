include_directories(../include)

add_executable(obts_smqueue smqueue.cpp poll.c QueuedMsgHdrs.cpp smcommands.cpp smnet.cpp SmqGlobals.cpp SmqMessageHandler.cpp SmqReader.cpp SmqWriter.cpp smsc.cpp SmqTest.cpp shortmsgpending.cpp shortmsg.cpp smqmanager.cpp shortcodeparams.cpp)

target_link_libraries(obts_smqueue commonlibs sms subreg nodemanager globals)
target_link_libraries(obts_smqueue zmq osip2 osipparser2 rt fmt enet)
