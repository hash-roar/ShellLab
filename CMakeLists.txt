cmake_minimum_required(VERSION 3.10)
set(CMAKE_EXPORT_COMPILE_COMMANDS on)

project(shell)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(mysh main.cc)
target_link_libraries(mysh shell)
target_include_directories(mysh PRIVATE ${PROJECT_SOURCE_DIR}/src)



#add library
add_subdirectory(src)

 # add tests
 add_subdirectory(test)