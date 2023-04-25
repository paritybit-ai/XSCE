
#project(PSI)

#############################################
#              Build PSI                    #
#############################################

file(GLOB_RECURSE SRCS ${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/source/oprf_psi/*.cpp)
file(GLOB_RECURSE SRCS_PSI ${CMAKE_CURRENT_SOURCE_DIR}/example/*.cpp)

add_library(PSI STATIC ${SRCS})
add_executable(psi ${SRCS_PSI})

add_library(psid SHARED ${SRCS})
add_executable(psid_test ${SRCS_PSI})

include_directories("./include")

target_link_libraries(PSI common toolkits libOTe SimplestOT cryptoTools sodium boost_system boost_thread)
target_link_libraries(psi common PIR PSI toolkits libOTe SimplestOT cryptoTools sodium boost_system boost_thread relic ${GRPC_LIBS} ssl crypto ${PUB_LIBS})
target_link_libraries(psid_test psid common toolkits libOTe SimplestOT cryptoTools sodium boost_system boost_thread relic ${GRPC_LIBS} ssl crypto ${PUB_LIBS})