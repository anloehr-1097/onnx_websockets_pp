#ifndef SRC_ONNXINFERLIB_INFERENCE_H
#define SRC_ONNXINFERLIB_INFERENCE_H
#include <onnxruntime_cxx_api.h>

#include <cassert>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/core/types.hpp>
#include <opencv2/opencv.hpp>
#include <string_view>
#include <vector>

#include "onnx_config.h"
#include "types.h"
#include "utils.h"

Ort::MemoryInfo get_mem_info(const std::string_view memtype);

class Yolov11Session {
  // Yolov11 Object Detection Session
 private:
  Ort::Env env;
  Ort::Session session{nullptr};
  const OnnxConfiguration &config;
  Ort::MemoryInfo memory_info;
  Ort::RunOptions run_opts;
  std::vector<int64_t> input_shape;
  size_t output_count = 1;  // TODO(andy) next refactoring --> config
  cv::Mat input_image{};
  std::vector<float> results;
  Ort::Value input_tensor{nullptr};
  Ort::Value output_tensor{nullptr};
  std::string_view input_names;
  std::string_view output_names;
  cv::Size img_size;

 public:
  Yolov11Session() = delete;
  // public initializer
  // call operator to feed image and obtain output
  Yolov11Session(std::filesystem::path model_path,
                 const OnnxConfiguration &conf);

  std::vector<ObbDetection> operator()(cv::Mat &img);

 private:
  void set_input_image(cv::Mat &img);
  std::vector<Ort::Value> detect();
  std::vector<ObbDetection> postprocess(std::vector<Ort::Value> &output,
                                        float thresh = 0.6);
  void set_run_opts() { assert(false && "Not Implemented yet."); }
};

#endif  // SRC_ONNXINFERLIB_INFERENCE_H
