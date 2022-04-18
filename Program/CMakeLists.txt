cmake_minimum_required(VERSION 3.21)
project(Program VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_EXTENSIONS NO)

set(CMAKE_CXX_FLAGS "-fno-exceptions -fno-rtti")

include_directories(include)

add_subdirectory(src src/build)
