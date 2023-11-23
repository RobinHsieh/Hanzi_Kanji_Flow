cmake_minimum_required(VERSION 3.23)
project(Hanzi_Kanji_Flow)

set(CMAKE_C_STANDARD 99)

add_executable(server src/server.c)

add_executable(client src/client.c)

find_package(Threads REQUIRED)

target_link_libraries(client PRIVATE Threads::Threads)