add_library(commonlibs BitVector.cpp Configuration.cpp LinkedLists.cpp Logger.cpp Reporting.cpp SelfDetect.cpp Sockets.cpp sqlite3util.cpp Threads.cpp Timeval.cpp UnixSignal.cpp URLEncode.cpp Utils.cpp Variables.cpp)

target_link_libraries(commonlibs pthread sqlite3)
