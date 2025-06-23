#include "onnx_config.h"
#include "utils.h"
#include <gtest/gtest.h>
#include <iostream>

TEST(CustOnnxConfig, TestCustOnnxConfigCreate) {
  std::cout << "--- Testing Custom Onnx Configuration!! ---";

  OnnxConfiguration &inf_conf = OnnxConfiguration::Config(
      640, 640, 3, 80, 1, "images", "output0", "yolov11obb", "cpu");

  OnnxConfiguration &new_conf = OnnxConfiguration::Config(
      640, 640, 3, 80, 1, "images", "output0", "yolov11obb", "cpu");

  ASSERT_EQ(&inf_conf, &new_conf);
}

