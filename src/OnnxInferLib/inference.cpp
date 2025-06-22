#include "include/inference.h"

#include <opencv2/core/hal/interface.h>

#include <cassert>
#include <cstddef>
#include <filesystem>
#include <functional>
#include <iostream>
#include <numeric>
#include <opencv2/core.hpp>
#include <opencv2/core/mat.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <typeinfo>
#include <vector>

#include "onnx_config.h"
#include "types.h"

Ort::MemoryInfo get_mem_info(std::string_view memtype) {
  if (memtype == "cpu") {
    return Ort::MemoryInfo::CreateCpu(OrtDeviceAllocator, OrtMemTypeCPU);
  } else {
    throw "GPU memory info not implemented yet.";
  }
}

Yolov11Session::Yolov11Session(std::filesystem::path model_path,
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
  run_opts = Ort::RunOptions();
}

void Yolov11Session::set_input_image(cv::Mat &img) {
  cv::Mat img_mat = preprocess_img(img, img_size, 0);
  assert(img_mat.type() == CV_32FC3 && "Image type must be CV_32FC3");
  assert(img_mat.isContinuous() && "Image must be in contiguous memory");
  // create 4d array from image and save into
  cv::dnn::blobFromImage(img_mat, input_image, 1.0, img_size, cv::Scalar(),
                         false, false);
}

std::vector<Ort::Value> Yolov11Session::detect() {
  // call detection on set input image

  // TODO(andy) in the future, these could be array of char strings
  // TODO(andy)
  assert((typeid(config.input_names()) == typeid(std::string_view(""))) &&
         "config.input_names() does not yield string_view");

  assert((typeid(config.output_names()) == typeid(std::string_view(""))) &&
         "config.output_names() does not yield string_view");

  const char *input_names[] = {config.input_names().data()};
  const char *output_names[] = {config.output_names().data()};

  Ort::Value input_tens = Ort::Value::CreateTensor<float>(
      memory_info, input_image.ptr<float>(), input_image.total(),
      input_shape.data(), input_shape.size());
  assert((input_shape.at(0) == 1) && "Check input shape at entry [0] == 1.");
  assert((output_count == 1) && "Check output_count == 1.");

  auto out_tens = session.Run(run_opts, input_names, &input_tens,
                              input_shape.at(0), output_names, output_count);
  return out_tens;
}

std::vector<ObbDetection> Yolov11Session::postprocess(
    std::vector<Ort::Value> &output, float thresh) {
  // Get the shape & data type of the tensor
  auto shape = output[0].GetTensorTypeAndShapeInfo().GetShape();
  ONNXTensorElementDataType type =
      output[0].GetTensorTypeAndShapeInfo().GetElementType();

  const void *raw_data = output[0].GetTensorData<void>();
  if (type == ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT) {
    const float *float_data = static_cast<const float *>(raw_data);
    size_t num_elements = std::accumulate(shape.begin(), shape.end(), 1,
                                          std::multiplies<size_t>());
    std::vector<ObbDetection> ret;
    ret.reserve(num_elements / 6);

    for (int f = 0; f < num_elements / 6; f++) {
      if (float_data[f * 6 + 4] > thresh) {
        ret.push_back(ObbDetection::from_c_array(float_data + (f * 6)));
      }
    }
    return ret;

  } else {
    return std::vector<ObbDetection>();
  }
}

std::vector<ObbDetection> Yolov11Session::operator()(cv::Mat &img) {
  set_input_image(img);
  std::vector<Ort::Value> det = detect();
  return postprocess(det);
}
