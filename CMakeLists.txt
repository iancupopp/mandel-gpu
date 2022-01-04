cmake_minimum_required(VERSION 3.21)
project(mandel)

set(CMAKE_CXX_STANDARD 20)

add_executable(mandel main.cpp glad.h glad.c stb_image.h)

find_package(glfw3 3.3 REQUIRED)
target_link_libraries(mandel glfw)
find_package(OpenGL REQUIRED)
target_link_libraries(mandel OpenGL::GL)

find_package(Threads REQUIRED)
target_link_libraries(mandel
        ${CMAKE_DL_LIBS}
        )


