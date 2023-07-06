include_directories(../include)

add_executable(obts_smqueue smqueue.cpp poll.c QueuedMsgHdrs.cpp smcommands.cpp smnet.cpp SmqGlobals.cpp SmqMessageHandler.cpp SmqReader.cpp SmqWriter.cpp smsc.cpp SmqTest.cpp shortmsgpending.cpp shortmsgpending.h shortmsg.cpp shortmsg.h smqmanager.cpp smqmanager.h shortcodeparams.cpp shortcodeparams.h)

target_link_libraries(obts_smqueue commonlibs sms subreg nodemanager globals)
target_link_libraries(obts_smqueue zmq osip2 osipparser2 rt)
