file(GLOB kqueue_srcs "${CMAKE_CURRENT_SOURCE_DIR}/*.cc")
add_library(kqueue 
    ${kqueue_srcs}
)
target_include_directories(kqueue
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
