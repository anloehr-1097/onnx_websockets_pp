cmake_minimum_required(VERSION 3.13)
if(WIN32)
  string(APPEND CMAKE_CXX_FLAGS " /W4")
else()
  string(APPEND CMAKE_CXX_FLAGS " -Wall -Wextra")
  string(APPEND CMAKE_C_FLAGS " -Wall -Wextra")
endif()

add_library(amqp conn_handler.cpp callbacks.cpp tasks.cpp amqp_utils.cpp amqp_socket.cpp)

# Open CV package to handle images
find_package(OpenCV REQUIRED)
target_include_directories(amqp PRIVATE ${OpenCV_INCLUDE_DIRS})

# AMQP library
target_include_directories(
    amqp PUBLIC "${PROJECT_SOURCE_DIR}/AMQP-CPP/include"
)
# hiredis library
message("Cmake Install prefix ${CMAKE_INSTALL_PREFIX}")
if(APPLE)
  message("Build on Apple.")
  target_include_directories(amqp PUBLIC "/opt/homebrew/Cellar/hiredis/1.3.0/include")
elseif(UNIX)
  message("Build on Linux.")
  target_include_directories(amqp PUBLIC
    "${CMAKE_INSTALL_PREFIX}/include/hiredis")
endif()

# target_include_directories(amqp PUBLIC "/opt/homebrew/Cellar/hiredis/1.3.0/include")

target_include_directories(
    amqp INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Tell CMake to search for the hiredis library
find_library(
  HIREDIS_LIBRARY         # The variable to store the result
  NAMES hiredis           # The name(s) of the library to find
  HINTS "${CMAKE_INSTALL_PREFIX}/lib"    # Optional: custom paths to search
  PATH_SUFFIXES lib       # Optional: look in lib subdirectories
)

# Check if the library was found
if(NOT HIREDIS_LIBRARY)
  message(FATAL_ERROR "hiredis library not found")
endif()

target_link_libraries(amqp PUBLIC ${HIREDIS_LIBRARY})

# Tell CMake to search for the amqp library
find_library(
  AMQP_CPP_LIBRARY         # The variable to store the result
  NAMES amqpcpp           # The name(s) of the library to find
  HINTS "${PROJECT_SOURCE_DIR}/AMQP-CPP/build/bin"
)

# Check if the library was found
if(NOT AMQP_CPP_LIBRARY)
  message(FATAL_ERROR "amqp-cpp library not found")
endif()

target_link_libraries(amqp PUBLIC ${AMQP_CPP_LIBRARY})
target_link_libraries(amqp PRIVATE OnnxInferLib)
target_link_libraries(amqp PRIVATE utils)
