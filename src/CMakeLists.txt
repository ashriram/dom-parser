# Source files (except the main for the executable):
file(GLOB SOURCES "src/*.cpp")
include(./gbench/GoogleBenchmark.cmake)

list(REMOVE_ITEM SOURCES "main.cpp")


# Compile the library
# add_library(${NEMO_LIB_NAME} ${NEMO_LIB_TYPE} ${nemo_src})

# Compile the executable
add_executable(dom_parser main.cpp)
add_executable(dom_bench bench.cpp)

set(BENCHMARK_NAMES layout parallel_layout test_rows test_cols test_task test_nested test_font test_textbox test_image) # ... add more names as needed


target_link_libraries(dom_bench benchmark::benchmark Threads::Threads)
target_link_libraries(dom_parser Threads::Threads)


# Link against FreeType

function(add_benchmark bm_name)
    add_executable(${bm_name} ${bm_name}.cpp)
    target_link_libraries(${bm_name} ${FREETYPE_LIBRARIES} ${PNG_LIBRARIES} ${BZIP2_LIBRARIES} harfbuzz::harfbuzz brotlienc brotlidec brotlicommon)
    add_dependencies(${bm_name} freetype brotli)
    target_link_libraries(${bm_name} benchmark::benchmark Threads::Threads)
endfunction()


# Iterate over the array and call the function:
foreach(bm IN LISTS BENCHMARK_NAMES)
    add_benchmark(${bm})
endforeach()