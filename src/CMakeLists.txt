# Source files (except the main for the executable):
file(GLOB SOURCES "src/*.cpp")

list(REMOVE_ITEM SOURCES "main.cpp")


# Compile the library
# add_library(${NEMO_LIB_NAME} ${NEMO_LIB_TYPE} ${nemo_src})

# Compile the executable
add_executable(dom_parser main.cpp)

target_link_libraries(dom_parser Threads::Threads)


# install(TARGETS nemo_exe RUNTIME DESTINATION bin)
# install(DIRECTORY ../include/nemo DESTINATION include)