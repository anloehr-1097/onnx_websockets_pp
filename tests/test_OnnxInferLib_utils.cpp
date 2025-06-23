#include <gtest/gtest.h>
#include <opencv2/core/hal/interface.h>

#include <cassert>
#include <filesystem>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/opencv.hpp>
#include <string>

#include "utils.h"
using namespace std::literals;

TEST(Onnx_Infer_Lib_Utils, Preprocess_Img) {
  const std::string img_path = "images/bus.jpg"s;
  const std::filesystem::path fpath(img_path);
  assert(std::filesystem::exists(fpath) && "Image path does not exist");
  cv::Mat mat = cv::imread(img_path);
  cv::Mat preprocessed = preprocess_img(mat, cv::Size(640, 640), false);
  double min, max;
  cv::minMaxLoc(preprocessed, &min, &max, NULL, NULL);

  ASSERT_EQ(preprocessed.rows, 640);
  ASSERT_EQ(preprocessed.cols, 640);
  ASSERT_EQ(preprocessed.channels(), 3);
  ASSERT_LE(max, 1.0);
  ASSERT_GE(min, 0.0);
  ASSERT_EQ(preprocessed.type(), CV_32FC3);
}
