# #onnxruntime providers
# # option(onnxruntime_USE_CUDA "Build with CUDA support" OFF)
# # option(onnxruntime_USE_TENSORRT "Build with TensorRT support" OFF)
# # option(LIBPNG_ROOTDIR "libpng root dir")
# # option(ONNXRUNTIME_ROOTDIR "onnxruntime root dir")

# if(NOT ONNXRUNTIME_ROOTDIR)
#   if(WIN32)
#     set(ONNXRUNTIME_ROOTDIR "C:/Program Files/onnxruntime")
# else()
#     # TODO make this more flexible
#     set(ONNXRUNTIME_ROOTDIR "/Users/anlhr/Projects/onnx_websockets/src/onnxruntime")
#   endif()
# endif()

# find_package(Asio REQUIRED)

# The ORT package has a different include directory structure to a local install via cmake.
# We added the path for the pre-built package above. Add the path for a local install to support either usage.
# TODO: If we want to support additional EPs being loadable from a local install we also need to add EP specific
# directories under /include/onnxruntime/core/providers
# include_directories("${ONNXRUNTIME_ROOTDIR}/include"                           # Pre-built package
#                     "${ONNXRUNTIME_ROOTDIR}/include/onnxruntime"               # Linux local install to /usr/local
#                     "${ONNXRUNTIME_ROOTDIR}/include/onnxruntime/core/session") # Windows local install
#
# link_directories("${ONNXRUNTIME_ROOTDIR}/lib")

find_package( OpenCV REQUIRED )
include_directories( ${OpenCV_INCLUDE_DIRS} )

add_executable(main main.cpp)
target_include_directories(main PUBLIC "/Users/anlhr/Projects/onnx_websockets/src/websocketpp" "/opt/homebrew/include")

# Define ASIO_STANDALONE for standalone Asio usage
target_compile_definitions(main PRIVATE ASIO_STANDALONE)
target_link_libraries(main PRIVATE pthread ${OpenCV_LIBS})
add_subdirectory(OnnxInferLib)
target_link_libraries(main PRIVATE OnnxInferLib)
