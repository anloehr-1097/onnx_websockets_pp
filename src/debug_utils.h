#include "onnxruntime_cxx_api.h"
#include <iostream>
#include <opencv2/core/hal/interface.h>
#include <opencv2/core/mat.hpp>
#ifndef UTILS.h
#define UTILS .h
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <string>
using namespace cv;

void save_image(cv::Mat &img, const char *fname) { imwrite(fname, img); }

cv::Mat image_uint_to_float(cv::Mat &img, float scale_val = 1.0 / 255.0) {
  cv::Mat res;
  img.convertTo(res, CV_32FC3, scale_val);
  return res;
}

cv::Mat image_float_to_uint(cv::Mat &img, float scale_val = 1.0) {
  cv::Mat res;
  img.convertTo(res, CV_8UC3, scale_val);
}

void inspect_tensor_data(Ort::Value &tens) {
  float *data = tens.GetTensorMutableData<float>();
  std::cout << "Data @ position 1: " << data[0];
}
#endif
