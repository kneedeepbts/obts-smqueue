add_library(subreg SubscriberRegistry.cpp servershare.cpp apps/sipauthserve.cpp)

target_link_libraries(subreg globals)
