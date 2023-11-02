file(GLOB_RECURSE SRCS_PSI_OFFLINE ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/oprf_psi/*.cpp)

find_package(Protobuf REQUIRED)
find_package(gRPC REQUIRED)

set(_PROTOBUF_LIBPROTOBUF protobuf::libprotobuf)
set(_REFLECTION gRPC::grpc++_reflection)
set(_GRPC_GRPCPP gRPC::grpc++)

SET(GRPC_LIBS ${_PROTOBUF_LIBPROTOBUF} ${_REFLECTION} ${_GRPC_GRPCPP})

include_directories("./")
include_directories("../..")
include_directories("/export/code/xsce/third_party/open-source-libs/include")
link_directories("/export/code/xsce/third_party/open-source-libs/lib64")

set(CMAKE_CXX_FLAGS  "${CMAKE_C_FLAGS}  -std=c++17")
set(CMAKE_CXX_STANDARD 17)
add_library(PSI_OFFLINE SHARED ${SRCS_PSI_OFFLINE})
add_executable(oprf_offline_test test/main.cpp)

target_link_libraries(PSI_OFFLINE libOTe cryptoTools boost_system boost_thread relic ${GRPC_LIBS} ssl crypto ${PUB_LIBS} -fPIC)
target_link_libraries(oprf_offline_test PSI_OFFLINE)