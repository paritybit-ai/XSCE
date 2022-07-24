
#project(pir)

#############################################
#              Build pir                    #
#############################################

file(GLOB_RECURSE SRCS ${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/example/*.cpp)
file(GLOB_RECURSE SRCS2 ${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp )
 
add_executable(pir ${SRCS})
add_library(PIR STATIC ${SRCS2})

include_directories("./include")

target_link_libraries(pir common PSI toolkits libOTe SimplestOT cryptoTools sodium boost_system boost_thread relic ssl crypto)
target_link_libraries(PIR common PSI toolkits libOTe SimplestOT cryptoTools sodium boost_system boost_thread relic ssl crypto)
