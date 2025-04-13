cmake_minimum_required(VERSION 3.13)
if (WIN32)
 string(APPEND CMAKE_CXX_FLAGS " /W4")
else()
 string(APPEND CMAKE_CXX_FLAGS " -Wall -Wextra")
 string(APPEND CMAKE_C_FLAGS " -Wall -Wextra")
endif()

# onnxruntime providers
option(onnxruntime_USE_CUDA "Build with CUDA support" OFF)
option(onnxruntime_USE_TENSORRT "Build with TensorRT support" OFF)
option(LIBPNG_ROOTDIR "libpng root dir")
option(ONNXRUNTIME_ROOTDIR "onnxruntime root dir")

include(FetchContent)

add_library(OnnxInferLib inference.cpp)

if(NOT ONNXRUNTIME_ROOTDIR)
  if(WIN32)
    set(ONNXRUNTIME_ROOTDIR "C:/Program Files/onnxruntime")
else()
    # TODO make this more flexible
    set(ONNXRUNTIME_ROOTDIR "/Users/anlhr/Projects/onnx_websockets/src/OnnxInferLib/onnxruntime")
  endif()
endif()
message("ONNXRUNTIME_ROOTDIR: ${ONNXRUNTIME_ROOTDIR}")

# The ORT package has a different include directory structure to a local install via cmake.
# We added the path for the pre-built package above. Add the path for a local install to support either usage.
# TODO: If we want to support additional EPs being loadable from a local install we also need to add EP specific
# directories under /include/onnxruntime/core/providers
include_directories("${ONNXRUNTIME_ROOTDIR}/include"                           # Pre-built package
                    "${ONNXRUNTIME_ROOTDIR}/include/onnxruntime"               # Linux local install to /usr/local
                    "${ONNXRUNTIME_ROOTDIR}/include/onnxruntime/core/session") # Windows local install

link_directories("${ONNXRUNTIME_ROOTDIR}/lib")

target_include_directories(OnnxInferLib PUBLIC 
    ${ONNXRUNTIME_ROOTDIR}/include
    ${ONNXRUNTIME_ROOTDIR}/include/onnxruntime
    ${ONNXRUNTIME_ROOTDIR}/include/onnxruntime/core/session
)

    
target_compile_definitions(OnnxInferLib PUBLIC ASIO_STANDALONE)
target_link_libraries(OnnxInferLib PUBLIC pthread "${ONNXRUNTIME_ROOTDIR}/lib/libonnxruntime.dylib" ${OpenCV_LIBS})
# target_include_directories(OnnxInferLib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# # anybody who uses this library needs to include the current source dir
target_include_directories(
    OnnxInferLib INTERFACE 
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# the public identifier makes the headers in include available under their names
# from main.cpp this allows us to '#include "inference.h"'.
target_include_directories(
    OnnxInferLib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include

)
