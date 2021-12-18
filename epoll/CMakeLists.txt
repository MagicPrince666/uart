file(GLOB epoll_srcs "${CMAKE_CURRENT_SOURCE_DIR}/*.cc")
add_library(multiplexing 
    ${epoll_srcs}
)
target_include_directories(multiplexing
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
