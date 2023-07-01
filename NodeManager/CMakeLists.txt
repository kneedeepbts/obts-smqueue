add_library(nodemanager NodeManager.cpp JsonBox-0.4.3/src/Array.cpp JsonBox-0.4.3/src/Convert.cpp JsonBox-0.4.3/src/Escaper.cpp JsonBox-0.4.3/src/IndentCanceller.cpp JsonBox-0.4.3/src/Indenter.cpp JsonBox-0.4.3/src/Object.cpp JsonBox-0.4.3/src/SolidusEscaper.cpp JsonBox-0.4.3/src/Value.cpp JSONDB/JSONDB.cpp)

target_link_libraries(nodemanager globals zmq)
