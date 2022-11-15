file(GLOB RNET_TEST_SOURCES "${PROJECT_SOURCE_DIR}/test/*test.cc")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(GTest REQUIRED)
find_package(Threads REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})
include_directories(${PROJECT_SOURCE_DIR}/src)
# build tests   
foreach(rnet_test_source ${RNET_TEST_SOURCES})
    # Create a human readable name.
    get_filename_component(rnet_test_filename ${rnet_test_source} NAME)
    string(REPLACE ".cc" "" rnet_test_name ${rnet_test_filename})
    
    #  
    add_executable(${rnet_test_name}  ${rnet_test_source})
    target_link_libraries(${rnet_test_name}  shell  ${GTEST_BOTH_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} gtest_main)
    
    set_target_properties(${rnet_test_name}
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/test"
    COMMAND ${rnet_test_name}
)


endforeach(rnet_test_source)