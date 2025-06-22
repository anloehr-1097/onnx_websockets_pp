#include <gtest/gtest.h>

#include <filesystem>
#include <vector>

#include "inference.h"
#include "onnx_config.h"
#include "test_YOLOv11SessionFixture.h"
#include "utils.h"

TEST(YOLOv11, TestYOLOv11SessionCreate) {
  OnnxConfiguration &yolo_config = OnnxConfiguration::Config(
      640, 640, 3, 80, 1, "images", "output0", "yolov11obb", "cpu");

  // specify model path & create model
  auto fp = std::filesystem::path{
      "/Users/anlhr/Projects/onnx_websockets/models/yolo11x_obb.onnx"};
  EXPECT_NO_THROW(Yolov11Session(fp, yolo_config));
  // ASSERT_EQ(res, 50);
}

TEST_F(Yolov11Fixture, TestInferenceResultType) {
  auto img = cv::imread("/Users/anlhr/Projects/onnx_websockets/images/bus.jpg",
                        cv::IMREAD_COLOR_RGB);

  auto res = onnx_sess(img);
  auto type_vec = std::vector<ObbDetection>();
  ASSERT_TRUE(typeid(res) == typeid(type_vec));
}

TEST_F(Yolov11Fixture, TestInferenceResultFormat) {
  auto img = cv::imread("/Users/anlhr/Projects/onnx_websockets/images/bus.jpg",
                        cv::IMREAD_COLOR_RGB);

  auto res = onnx_sess(img);
  auto first_elem = res.at(0);
  ASSERT_EQ(first_elem.to_vec().size(), 6);
}
