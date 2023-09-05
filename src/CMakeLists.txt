# Source files (except the main for the executable):
file(GLOB SOURCES "src/*.cpp")
include(./gbench/GoogleBenchmark.cmake)

list(REMOVE_ITEM SOURCES "main.cpp")


# Compile the library
# add_library(${NEMO_LIB_NAME} ${NEMO_LIB_TYPE} ${nemo_src})

# Compile the executable
add_executable(dom_parser main.cpp)
add_executable(dom_bench bench.cpp)

set(BENCHMARK_NAMES layout parallel_layout test_rows test_cols test_task test_nested test_font) # ... add more names as needed

# add_executable(layout layout.cpp)
# add_executable(parallel_layout parallel_layout.cpp)
# add_executable(test_rows test_rows.cpp)
# add_executable(test_cols test_cols.cpp)
# add_executable(test_task test_task.cpp)
# add_executable(test_nested test_nested.cpp)

target_link_libraries(dom_bench benchmark::benchmark Threads::Threads)
target_link_libraries(dom_parser Threads::Threads)

# install(TARGETS nemo_exe RUNTIME DESTINATION bin)
# install(DIRECTORY ../include/nemo DESTINATION include)
include(ExternalProject)

# Lib freetype
# Define FreeType as an external project
ExternalProject_Add(
    freetype
    PREFIX ${CMAKE_BINARY_DIR}/_deps/freetype
    URL https://download.savannah.gnu.org/releases/freetype/freetype-2.10.4.tar.gz
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
)

# Define the install location for FreeType
set(FREETYPE_INSTALL_DIR ${CMAKE_BINARY_DIR}/_deps/freetype)
# Define FreeType include and lib directories
set(FREETYPE_INCLUDE_DIRS ${FREETYPE_INSTALL_DIR}/include/freetype2)
set(FREETYPE_LIBRARIES ${FREETYPE_INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}freetype${CMAKE_STATIC_LIBRARY_SUFFIX})

# Include and link directories
include_directories(${FREETYPE_INCLUDE_DIRS})
link_directories(${FREETYPE_INSTALL_DIR}/lib)

FIND_PACKAGE(PNG REQUIRED)
INCLUDE_DIRECTORIES(${PNG_INCLUDE_DIRS})
LINK_DIRECTORIES(${PNG_LIBRARY_DIRS})


find_package(BZip2 REQUIRED)
INCLUDE_DIRECTORIES(${BZIP2_INCLUDE_DIRS})
LINK_DIRECTORIES(${BZIP2_LIBRARY_DIRS})


find_package(harfbuzz CONFIG REQUIRED)
get_target_property(HARFBUZZ_INCLUDE_DIRS harfbuzz::harfbuzz INTERFACE_INCLUDE_DIRECTORIES)

# add_dependencies(YourProjectName freetype)
# Lib freetype
# Define FreeType as an external project
ExternalProject_Add(
    brotli
    PREFIX ${CMAKE_BINARY_DIR}/_deps/brotli
    GIT_REPOSITORY https://github.com/google/brotli.git
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
)

# Define the install location for FreeType
set(BROTLI_INSTALL_DIR ${CMAKE_BINARY_DIR}/_deps/brotli)


# Include and link directories
include_directories(${BROTLI_INSTALL_DIR}/include/)
link_directories(${BROTLI_INSTALL_DIR}/lib)





# Link against FreeType

function(add_benchmark bm_name)
    add_executable(${bm_name} ${bm_name}.cpp)
    target_link_libraries(${bm_name} ${FREETYPE_LIBRARIES} ${PNG_LIBRARIES} ${BZIP2_LIBRARIES} harfbuzz::harfbuzz brotlienc brotlidec brotlicommon)
endfunction()

find_package(PNG REQUIRED)


# Iterate over the array and call the function:
foreach(bm IN LISTS BENCHMARK_NAMES)
    add_benchmark(${bm})
endforeach()