cmake_minimum_required(VERSION 3.28)
project(Ex1 C)

set(CMAKE_C_STANDARD 11)

add_executable(
        Ex1
        ex1.c
        ex2.c
        shell.c
        Ex3q1.c
)
# Add this to your CMakeLists.txt
target_link_libraries(Ex1 m)
