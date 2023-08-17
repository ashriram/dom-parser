# Source files (except the main for the executable):
file(GLOB SOURCES "src/*.cpp")
include(./gbench/GoogleBenchmark.cmake)

list(REMOVE_ITEM SOURCES "main.cpp")


# Compile the library
# add_library(${NEMO_LIB_NAME} ${NEMO_LIB_TYPE} ${nemo_src})

# Compile the executable
add_executable(dom_parser main.cpp)
add_executable(dom_bench bench.cpp)
add_executable(layout layout.cpp)
add_executable(parallel_layout parallel_layout.cpp)
add_executable(test_rows test_rows.cpp)
add_executable(test_cols test_cols.cpp)
target_link_libraries(dom_bench benchmark::benchmark Threads::Threads)
target_link_libraries(dom_parser Threads::Threads)


# install(TARGETS nemo_exe RUNTIME DESTINATION bin)
# install(DIRECTORY ../include/nemo DESTINATION include)