cmake_minimum_required(VERSION 3.10)
project(hf_trading)

# Set C++ Standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add include directories
include_directories(src/include)

# Add library sources
file(GLOB LIB_SOURCES src/lib/*.cpp)

# Add executable
add_executable(hf_trading src/main.cpp ${LIB_SOURCES})

# Find and link libcurl
find_package(CURL REQUIRED)
target_link_libraries(hf_trading PRIVATE CURL::libcurl)

# Find and link GTK
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK REQUIRED gtk+-3.0)

include_directories(${GTK_INCLUDE_DIRS})
link_directories(${GTK_LIBRARY_DIRS})
add_definitions(${GTK_CFLAGS_OTHER})

target_link_libraries(hf_trading PRIVATE ${GTK_LIBRARIES})

# Find and link JsonCpp
find_package(jsoncpp REQUIRED)
target_link_libraries(hf_trading PRIVATE JsonCpp::JsonCpp)

# Optional: Install target
install(TARGETS hf_trading DESTINATION bin)
