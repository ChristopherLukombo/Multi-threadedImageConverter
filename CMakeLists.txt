cmake_minimum_required(VERSION 3.10)
project(Multi_threadedImageConverter C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS -pthread)


add_executable(Multi_threadedImageConverter edge-detect.c bitmap.c)