#include "inference.h"
#include "utils.h"
#include <filesystem>
#include <gtest/gtest.h>

TEST(YOLOv11, TestYOLOv11SessionRun) {

  // specify model path & create model
  auto fp = std::filesystem::path{
      "/Users/anlhr/Projects/onnx_websockets/models/yolo11x_obb.onnx"};
  auto onnx_sess = Yolov11Session(fp);

  // read image from disk & preprocess image
  auto img = cv::imread("/Users/anlhr/Projects/onnx_websockets/images/bus.jpg",
                        cv::IMREAD_COLOR_RGB);
  img = preprocess_img(img, cv::Size(640, 640), 0);
  onnx_sess.set_input_image(img);
  auto onnx_out_tens = onnx_sess.detect();
  auto res = onnx_sess.postprocess(onnx_out_tens);
  ASSERT_TRUE(typeid(res) == typeid(std::vector<float>()));
  // ASSERT_EQ(res, 50);
}
