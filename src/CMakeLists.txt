
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

# include(FetchContent)
# requests lib similar to python requests
# JSON parsing lib
# FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz)
# FetchContent_MakeAvailable(json)

# find_package(OpenCV REQUIRED)
# include_directories(${OpenCV_INCLUDE_DIRS})
# include_directories("/Users/anlhr/Projects/AMQP-CPP/include")


# add_executable(main main.cpp callbacks.cpp json_utils.cpp base64.cpp tasks.cpp)
add_executable(main main.cpp)
target_include_directories(main PUBLIC  "/opt/homebrew/include")

# Define ASIO_STANDALONE for standalone Asio usage
# target_compile_definitions(main PRIVATE ASIO_STANDALONE)
# target_link_libraries(main PRIVATE pthread ${OpenCV_LIBS} )
# target_link_libraries(main PUBLIC "/Users/anlhr/Projects/AMQP-CPP/build/bin/libamqpcpp.dylib")
add_subdirectory(OnnxInferLib)
add_subdirectory(amqp)
add_subdirectory(utils)

target_link_libraries(main PRIVATE OnnxInferLib)
target_link_libraries(main PRIVATE amqp)
target_link_libraries(main PUBLIC nlohmann_json::nlohmann_json)
