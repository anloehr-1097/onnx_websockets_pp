cmake_minimum_required(VERSION 3.15...3.29)
project(onnx_websockets LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(utils json_utils.cpp base64.cpp)

# JSON parsing lib
include(FetchContent)
fetchcontent_declare(json URL 
https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz)
fetchcontent_makeavailable(json)
target_link_libraries(utils PUBLIC nlohmann_json::nlohmann_json)


target_include_directories(
    utils PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

find_package(OpenCV REQUIRED)
target_include_directories(utils PRIVATE ${OpenCV_INCLUDE_DIRS})
target_link_libraries(utils PRIVATE ${OpenCV_LIBS} )
