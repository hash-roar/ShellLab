cmake_minimum_required(VERSION 3.10)
set(CMAKE_EXPORT_COMPILE_COMMANDS on)

project(shell)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/shlab-handout)

file(GLOB SRCS ./*.cc)
message(${SRCS})
add_executable(tsh ${SRCS})
target_include_directories(tsh PRIVATE ${PROJECT_SOURCE_DIR}/)
