file(GLOB kqueue_srcs "${CMAKE_CURRENT_SOURCE_DIR}/*.cc")
add_library(multiplexing 
    ${kqueue_srcs}
)
target_include_directories(multiplexing
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
