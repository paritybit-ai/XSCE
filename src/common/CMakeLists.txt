
#project(PSI)

#############################################
#              Build PSI                    #
#############################################

file(GLOB_RECURSE SRCS ${CMAKE_CURRENT_SOURCE_DIR}/pub/source/*.cpp)

add_library(common STATIC ${SRCS})

include_directories("./pub/include")

target_compile_options(common PUBLIC -fPIC)
target_link_libraries(common libOTe cryptoTools boost_system boost_thread)
