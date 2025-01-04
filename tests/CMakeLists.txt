cmake_minimum_required(VERSION 3.10)
project(hf_trading)

# Set C++ Standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add include directories

# Add library sources
file(GLOB LIB_SOURCES src/lib/*.cpp)

# Add executable
add_executable(data_gen dev_data_generator.cpp ${LIB_SOURCES})
