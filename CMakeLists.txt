cmake_minimum_required(VERSION 3.10)
project(myShell C)

set(CMAKE_C_STANDARD 99)
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address -g")


add_executable(myShell main.c cmd_storage.h cmd_storage.c RBTree.h RBTree.c stack.h stack.c)

target_link_libraries(myShell pthread)