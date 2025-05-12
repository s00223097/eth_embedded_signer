cmake_minimum_required(VERSION 3.10)
project(eth_embedded_signer C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")

# Include directories
include_directories(include)

# Source files
file(GLOB SOURCES "src/*.c")

# Main executable
add_executable(eth_signer ${SOURCES})

# Tests executable
file(GLOB TEST_SOURCES "tests/*.c")
add_executable(run_tests ${TEST_SOURCES} ${SOURCES}) 