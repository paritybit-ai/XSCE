
cmake_minimum_required (VERSION 3.20)

project(XSCE VERSION 1.0)

# Set a default build type for single-configuration
# CMake generators if no build type is set.
if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
	SET(CMAKE_BUILD_TYPE Release)
endif()

add_definitions(-DENABLE_BOOST_OPENSSL)
#add_definitions(-DENABLE_WOLFSSL)
if (NOT THIRD_LIB)
	set(THIRD_LIB_ROOT ${PROJECT_SOURCE_DIR}/third_party/libOTe_libs)
else()
	set(THIRD_LIB_ROOT ${PROJECT_SOURCE_DIR}/${THIRD_LIB})
endif()
set(THIRD_LIB_PATH ${THIRD_LIB_ROOT}/lib)
set(THIRD_LIB64_PATH ${THIRD_LIB_ROOT}/lib64)
set(PROJECT_LIB_PATH ${PROJECT_SOURCE_DIR}/lib)
set(THIRD_LIB_CMAKE_PATH ${THIRD_LIB_PATH}/cmake)
set(THIRD_LIB64_CMAKE_PATH ${THIRD_LIB64_PATH}/cmake)

message("THIRD_LIB_ROOT = ${THIRD_LIB_ROOT}")

#############################################
#           Set Base Variables              #
#############################################

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_HOME_DIRECTORY}/build/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_HOME_DIRECTORY}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_HOME_DIRECTORY}/lib)

set(CMAKE_C_FLAGS "-Wall -maes -msse2 -msse4.1 -msse3 -mavx -mavx2 -mpclmul  -pthread -Wno-ignored-attributes -Wno-int-in-bool-context -no-pie")
set(CMAKE_CXX_FLAGS  "${CMAKE_C_FLAGS}  --std=c++14 -DBOOST_BIND_GLOBAL_PLACEHOLDERS")

# Select flags.
if ("${CMAKE_BUILD_TYPE}" STREQUAL "Release")
	SET(CMAKE_C_FLAGS_RELEASE "-O2 -DNDEBUG") 
	SET(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG") 
elseif ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
	SET(CMAKE_C_FLAGS_DEBUG  "-O0 -g -ggdb -rdynamic")
	SET(CMAKE_CXX_FLAGS_DEBUG  "-O0 -g -ggdb -rdynamic")
elseif ("${CMAKE_BUILD_TYPE}" STREQUAL  "RELWITHDEBINFO")
	SET(CMAKE_C_FLAGS_RELWITHDEBINFO " -O2 -g -ggdb -rdynamic")
	SET(CMAKE_CXX_FLAGS_RELWITHDEBINFO " -O2 -g -ggdb -rdynamic")
else()
	message(ERROR ": Unknown build type - ${CMAKE_BUILD_TYPE}  Please use one of Debug, Release, or RELWITHDEBINFO\n\n" )
endif()

#############################################
#       Library/includes locations          #
#############################################

include_directories(./)
include_directories(./src)
include_directories(${THIRD_LIB_ROOT}/include)

link_directories(./lib)
link_directories(${THIRD_LIB64_PATH})
link_directories(${THIRD_LIB_PATH})

add_subdirectory(src/common)
add_subdirectory(src/toolkits)
add_subdirectory(src/PSI)
add_subdirectory(src/pir)
