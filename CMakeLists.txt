CMAKE_MINIMUM_REQUIRED(VERSION 3.1)
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
option(EXAMPLE "shall we complie example" on)
#INCLUDE(build_for_r328)
#INCLUDE(build_for_rk3308)

include_directories(${CMAKE_SOURCE_DIR})

PROJECT(CppDemo)

# SET(CMAKE_BUILD_TYPE "Debug")
SET(CMAKE_BUILD_TYPE "Release")

SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS} -O0 -Wall -g")
SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS} -Os -Wall")

SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -std=c++14 -O0 -Wall -g")
SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -std=c++14 -Os -Wall")

MESSAGE(STATUS ${CMAKE_SYSTEM_NAME})

IF (WIN32)
	MESSAGE(STATUS "Now is windows.")
    add_subdirectory(iocp)
ELSEIF (APPLE)
	MESSAGE(STATUS "Now is Apple systens.")
    add_subdirectory(kqueue)
ELSEIF (UNIX)
	MESSAGE(STATUS "Now is UNIX-like OS's.")
    add_subdirectory(epoll)
ENDIF ()

set(EXEC_NAME cxxdemo)

add_executable(${EXEC_NAME} main.cc
    uart.cc
)

target_link_libraries(${EXEC_NAME} 

)

