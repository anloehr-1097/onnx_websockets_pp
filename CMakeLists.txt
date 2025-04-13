cmake_minimum_required(VERSION 3.15...3.29)
project(onnx_websockets LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# add_definitions(-D_GLIBCXX_USE_CXX11_ABI=0)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_VISIBILITY_PRESET hidden) 

message("System / OS: ${CMAKE_SYSTEM_NAME}")

add_subdirectory(src)
add_subdirectory(tests)

