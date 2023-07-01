add_library(sms SMSTransfer.cpp SMSMessages.cpp)

target_link_libraries(sms gsm)