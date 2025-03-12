cmake_minimum_required(VERSION 3.15...3.29)
project(onnx_websockets LANGUAGES CXX)

if (WIN32)
 string(APPEND CMAKE_CXX_FLAGS " /W4")
else()
 string(APPEND CMAKE_CXX_FLAGS " -Wall -Wextra")
 string(APPEND CMAKE_C_FLAGS " -Wall -Wextra")
endif()

#onnxruntime providers
option(onnxruntime_USE_CUDA "Build with CUDA support" OFF)
option(onnxruntime_USE_TENSORRT "Build with TensorRT support" OFF)
option(LIBPNG_ROOTDIR "libpng root dir")
option(ONNXRUNTIME_ROOTDIR "onnxruntime root dir")
include(FetchContent)

set(CMAKE_CXX_STANDARD 17)
# add_definitions(-D_GLIBCXX_USE_CXX11_ABI=0)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_VISIBILITY_PRESET hidden) 
# setting torch and eigen install locations depending on system 
message("System / OS: ${CMAKE_SYSTEM_NAME}")
# IF(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
# 	# TODO change these
# 	set(TORCH_INSTALL_PREFIX "/opt/venv/lib/python3.12/site-packages/torch")
# 	set(EIGEN3_INCLUDE_DIR "${HOME}/opt/Eigen")
# 	set(Eigen3_DIR "${HOME}/opt/Eigen/share/eigen3/cmake")
# 	set (pybind11_DIR "/opt/venv/lib/python3.12/site-packages/pybind11/share/cmake/pybind11")
#
# ELSE()
# 	set(TORCH_INSTALL_PREFIX "/opt/homebrew/Caskroom/miniconda/base/envs/ML-practice/lib/python3.12/site-packages/torch")
# 	set(EIGEN3_INCLUDE_DIR "${HOME}/opt/Eigen")
# 	set(Eigen3_DIR "${HOME}/opt/Eigen/share/eigen3/cmake")
# 	set (pybind11_DIR "/opt/homebrew/Caskroom/miniconda/base/envs/ML-practice/lib/python3.12/site-packages/pybind11/share/cmake/pybind11")
# ENDIF()

# set(TORCH_INSTALL_PREFIX "/opt/homebrew/Caskroom/miniconda/base/envs/ML-practice/lib/python3.12/site-packages/torch")
# set(EIGEN3_INCLUDE_DIR "${HOME}/opt/Eigen")
# set(Eigen3_DIR "${HOME}/opt/Eigen/share/eigen3/cmake")

# list(APPEND CMAKE_PREFIX_PATH ${TORCH_INSTALL_PREFIX})
# list(APPEND CMAKE_PREFIX_PATH ${TORCH_INSTALL_PREFIX})
# # list(APPEND CMAKE_PREFIX_PATH "${HOME}/opt/Eigen/include")
# list(APPEND CMAKE_PREFIX_PATH "${HOME}/opt/Eigen/include/eigen3")

# find_package(Torch REQUIRED)
# find_package(Python REQUIRED COMPONENTS Interpreter Development)
# find_package(Eigen3 3.4.0 REQUIRED NO_MODULE)
# include_directories(${TORCH_INCLUDE_DIRS})
# include_directories(${PYTHON_INCLUDE_DIRS})

# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${TORCH_CXX_FLAGS}")
# find_library(TORCH_PYTHON_LIBRARY torch_python "${TORCH_INSTALL_PREFIX}/lib")
# message(TORCH_PYTHON_LIBRARY="${TORCH_PYTHON_LIBRARY}")
#
# set (pybind11_DIR "/opt/homebrew/Caskroom/miniconda/base/envs/ML-practice/lib/python3.12/site-packages/pybind11/share/cmake/pybind11")
# set(PYBIND11_FINDPYTHON ON)
# find_package(pybind11 CONFIG REQUIRED)
if(NOT ONNXRUNTIME_ROOTDIR)
  if(WIN32)
    set(ONNXRUNTIME_ROOTDIR "C:/Program Files/onnxruntime")
else()
    set(ONNXRUNTIME_ROOTDIR "/Users/anlhr/Projects/onnx_websockets/onnxruntime")
  endif()
endif()

# find_package(Asio REQUIRED)

# The ORT package has a different include directory structure to a local install via cmake.
# We added the path for the pre-built package above. Add the path for a local install to support either usage.
# TODO: If we want to support additional EPs being loadable from a local install we also need to add EP specific
# directories under /include/onnxruntime/core/providers
include_directories("${ONNXRUNTIME_ROOTDIR}/include"                           # Pre-built package
                    "${ONNXRUNTIME_ROOTDIR}/include/onnxruntime"               # Linux local install to /usr/local
                    "${ONNXRUNTIME_ROOTDIR}/include/onnxruntime/core/session") # Windows local install

link_directories("${ONNXRUNTIME_ROOTDIR}/lib")

# Link pthread for multithreading support
# pybind11_add_module(np_interop np_interop.cpp)
add_executable(main main.cpp)
target_include_directories(main PUBLIC "/Users/anlhr/Projects/onnx_websockets/websocketpp" "/opt/homebrew/include" "/Users/anlhr/Projects/onnx_websockets/onnxruntime/include")

# Define ASIO_STANDALONE for standalone Asio usage
target_compile_definitions(main PRIVATE ASIO_STANDALONE)
target_link_libraries(main PRIVATE pthread)

# target_link_libraries(np_interop PRIVATE ${TORCH_LIBRARIES} ${TORCH_PYTHON_LIBRARY} Eigen3::Eigen)
# # target_link_libraries(np_interop PRIVATE ${TORCH_LIBRARIES} ${TORCH_PYTHON_LIBRARY})
# set_property(TARGET np_interop PROPERTY CXX_STANDARD 17)
# install(TARGETS np_interop DESTINATION .)




