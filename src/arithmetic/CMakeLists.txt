
#project(arithmetic)

#############################################
#              Build arithmetic                    #
#############################################

set(CMAKE_C_FLAGS "-DUSE_KOS -DUSE_GF2N_LONG -march=native -Wextra -Wall -pthread -Werror -fPIC -MMD -MP -msse4.1 -msse4.2 -maes -mpclmul -mavx -mavx2 -mbmi2 -madx")
set(CMAKE_CXX_FLAGS  "${CMAKE_C_FLAGS}  --std=c++14")

SET(CMAKE_C_FLAGS_RELEASE "-O3") 
SET(CMAKE_CXX_FLAGS_RELEASE "-O3") 

file(GLOB_RECURSE SRCS ${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/example/*.cpp)
file(GLOB_RECURSE SRCS2 ${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp )

add_executable(arithmetic ${SRCS})
add_library(arith STATIC ${SRCS2})

add_definitions(-w)

include_directories("./include")

target_link_libraries(arith MP-SPDZ mpirxx mpir simpleot SimplestOT_spdz ssl crypto)
target_link_libraries(arithmetic arith MP-SPDZ mpirxx mpir simpleot SimplestOT_spdz sodium ssl crypto boost_system boost_thread)
