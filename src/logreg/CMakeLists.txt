
#project(pir)

#############################################
#              Build pir                    #
#############################################

file(GLOB_RECURSE SRCS ${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/example/main.cpp)
# file(GLOB_RECURSE SRCS2 ${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/example/main2.cpp)

#disable warning output
add_definitions(-w)

add_executable(lr ${SRCS})
# add_library(liblr STATIC ${SRCS2})

include_directories("./include")

target_link_libraries(lr  arith common PSI toolkits libOTe  cryptoTools sodium boost_system boost_thread relic ${GRPC_LIBS}  ssl crypto)
# target_link_libraries(liblr common PSI toolkits libOTe SimplestOT cryptoTools sodium boost_system boost_thread relic ssl crypto)
