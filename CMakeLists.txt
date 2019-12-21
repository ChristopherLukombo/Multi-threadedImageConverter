cmake_minimum_required(VERSION 3.10)
project(Multi_threadedImageConverter C)

set(CMAKE_C_STANDARD 99)

add_executable(Multi_threadedImageConverter edge-detect.c bitmap.c)