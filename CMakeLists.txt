cmake_minimum_required(VERSION 3.3)
project(main)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Ofast -std=c++11 ")

set(SOURCE_FILES mandel.cpp)

add_executable(main ${SOURCE_FILES})

LINK_DIRECTORIES(/usr/lib/x86_64-linux-gnu/)
TARGET_LINK_LIBRARIES(main glfw GLEW GLU GL pthread)

