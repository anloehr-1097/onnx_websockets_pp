cmake_minimum_required(VERSION 3.13)
if(WIN32)
  string(APPEND CMAKE_CXX_FLAGS " /W4")
else()
  string(APPEND CMAKE_CXX_FLAGS " -Wall -Wextra")
  string(APPEND CMAKE_C_FLAGS " -Wall -Wextra")
endif()

# TODO
add_library(amqp amqp.cpp callbacks.cpp tasks.cpp)
# Open CV package to handle images
find_package(OpenCV REQUIRED)
target_include_directories(amqp PRIVATE ${OpenCV_INCLUDE_DIRS})
target_include_directories(
    amqp PRIVATE "/Users/anlhr/Projects/AMQP-CPP/include"
)
target_include_directories(
    amqp INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)
# target_include_directories(amqp PRIVATE "../OnnxInferLib/include/inference.h")
target_link_libraries(amqp
    PRIVATE "/Users/anlhr/Projects/AMQP-CPP/build/bin/libamqpcpp.dylib"
)

target_link_libraries(amqp PRIVATE OnnxInferLib)
target_link_libraries(amqp PRIVATE utils)

# the public identifier makes the headers in include available under their names
# from main.cpp this allows us to '#include "inference.h"'.
# target_include_directories(
#     OnnxInferLib PUBLIC
#     ${CMAKE_CURRENT_SOURCE_DIR}/include
#
# )
