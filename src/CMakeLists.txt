file(GLOB SRCS ./*.cc)

add_library(shell ${SRCS})
target_include_directories(shell PUBLIC ${PROJECT_SOURCE_DIR}/src)