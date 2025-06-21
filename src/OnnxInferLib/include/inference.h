#ifndef SRC_ONNXINFERLIB_INFERENCE_H
#define SRC_ONNXINFERLIB_INFERENCE_H
#include "onnx_config.h"
#include "onnxruntime_c_api.h"
#include "types.h"
#include "utils.h"
#include <cstdint>
#include <filesystem>
#include <functional>
#include <iostream>
#include <memory>
#include <numeric>
#include <onnxruntime_cxx_api.h>
#include <opencv2/core.hpp>
#include <opencv2/core/types.hpp>
#include <opencv2/opencv.hpp>
#include <string_view>
#include <vector>

Ort::MemoryInfo get_mem_info(const std::string_view memtype);

class Yolov11Session {
  // Yolov11 Object Detection Session
private:
  Ort::Env env;
  Ort::Session session{nullptr};
  const OnnxConfiguration &config;
  Ort::MemoryInfo memory_info;
  std::vector<int64_t> input_shape;
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
  Yolov11Session(std::filesystem::path model_path,
                 const OnnxConfiguration &conf)
      : config(conf), memory_info(get_mem_info(conf.device())) {
    // init session if filepath exists
    if (std::filesystem::exists(model_path)) {
      session =
          Ort::Session(env, model_path.c_str(), Ort::SessionOptions{nullptr});
    } else {
      std::cerr << "Model path " << model_path << " does not exist. Exiting ..";
      exit(1);
    }
    input_shape = {1, conf.input_channels(), conf.input_width(),
                   conf.input_height()};

    img_size = cv::Size(config.input_width(), config.input_height());
  }
  // call operator to feed image and obtain output
  std::vector<ObbDetection> operator()(cv::Mat &img);

private:
  void set_input_image(cv::Mat &img);
  std::vector<Ort::Value> detect();
  std::vector<ObbDetection> postprocess(std::vector<Ort::Value> &output,
                                        float thresh = 0.6);
};

#endif // SRC_ONNXINFERLIB_INFERENCE_H
