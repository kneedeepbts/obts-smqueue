cmake_minimum_required(VERSION 3.25)
project(obts_smqueue)

set(CMAKE_CXX_STANDARD 20)

include_directories(include CommonLibs Globals GSM NodeManager NodeManager/JsonBox-0.4.3/include NodeManager/JSONDB smqueue SMS SR)

add_subdirectory(CommonLibs)
add_subdirectory(Globals)
add_subdirectory(GSM)
add_subdirectory(NodeManager)
add_subdirectory(smqueue)
add_subdirectory(SMS)
add_subdirectory(SR)
