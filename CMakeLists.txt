cmake_minimum_required(VERSION 3.5)

project(DOMParser)

# Set the C++ standard to C++11
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
include_directories(include)

set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)

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

# #Harfbuzz
# ExternalProject_Add(
# harfbuzz
# PREFIX ${CMAKE_BINARY_DIR}/_deps/harfbuzz
# GIT_REPOSITORY https://github.com/harfbuzz/harfbuzz.git
# GIT_TAG main
# CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
# )
# set(HARFBUZZ_INSTALL_DIR ${CMAKE_BINARY_DIR}/_deps/harfbuzz)
# include_directories(${HARFBUZZ_INSTALL_DIR2}/include/)
# link_directories(${HARFBUZZ_INSTALL_DIR2}/lib)
# # Define FreeType include and lib directories
# set(HARFBUZZ_LIBRARIES ${HARFBUZZ_INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}harfbuzz${CMAKE_STATIC_LIBRARY_SUFFIX})





# Brotli
ExternalProject_Add(
    brotli
    PREFIX ${CMAKE_BINARY_DIR}/_deps/brotli
    GIT_REPOSITORY https://github.com/google/brotli.git
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
)
set(BROTLI_INSTALL_DIR ${CMAKE_BINARY_DIR}/_deps/brotli)
include_directories(${BROTLI_INSTALL_DIR}/include/)
link_directories(${BROTLI_INSTALL_DIR}/lib)




add_subdirectory(src)
