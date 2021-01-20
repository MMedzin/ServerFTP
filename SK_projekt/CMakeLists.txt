cmake_minimum_required(VERSION 3.17)
project(SK_projekt C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS -pthread)

add_executable(SK_projekt main.c utils.h utils.c command_parser.h command_parser.c commands.h commands.c thread_data_t.h)