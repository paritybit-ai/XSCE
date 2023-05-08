
PROJECT(toolkits)

include_directories(.)
include_directories(./util/include)

aux_source_directory(util/source/ DIR_SRCS)

add_library(toolkits STATIC ${DIR_SRCS})

target_compile_options(toolkits PUBLIC -fPIC)
target_link_libraries(toolkits )
