enable_testing()
# --- Google Test ---
# Download and unpack googletest at configure time
include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG 52204f78f94d7512df1f0f3bea1d47437a2c3a58)
FetchContent_MakeAvailable(googletest)

add_executable(cust_test test_Yolov11Session.cpp test_ConnHandler.cpp test_ConnHandlerFixture.cpp test_CustOnnxConfig.cpp)
target_link_libraries(cust_test PRIVATE
  GTest::gtest_main
  OnnxInferLib
  amqp
  utils)

# add_test(cust_test_gtests cust_test)  # add test to run manually
include(GoogleTest)
gtest_discover_tests(cust_test)  # instead of adding manually, let discover

# navigate to build directory for tests and run 'ctest' to run all tests
