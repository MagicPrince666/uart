file(GLOB epoll_srcs "${CMAKE_CURRENT_SOURCE_DIR}/*.cc")
add_library(xepoll 
    ${epoll_srcs}
)
target_include_directories(xepoll
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
