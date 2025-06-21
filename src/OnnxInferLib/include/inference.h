#ifndef SRC_ONNXINFERLIB_INFERENCE_H
#define SRC_ONNXINFERLIB_INFERENCE_H
#include "onnx_config.h"
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

Ort::MemoryInfo get_mem_info(std::string_view memtype);

struct Yolov11Session {
  // Yolov11 Object Detection Session
private:
  Ort::Env env;
  Ort::Session session{nullptr};
  const OnnxConfiguration &config;
  Ort::MemoryInfo memory_info;

  std::vector<int64_t> input_shape;

public:
  // Yolov11 inference session

  cv::Mat input_image{};
  std::vector<float> results;
  Ort::Value input_tensor{nullptr};
  Ort::Value output_tensor{nullptr};
  std::string_view input_names;
  std::string_view output_names;
  int64_t result{0};
  cv::Size img_size;

  Yolov11Session(std::filesystem::path model_path,
                 const OnnxConfiguration &conf)
      : config(conf), memory_info(get_mem_info(conf.device())) {

    // init session if filepath exists
    if (std::filesystem::exists(model_path)) {
      session =
          Ort::Session(env, model_path.c_str(), Ort::SessionOptions{nullptr});
    } else {
      std::cerr << "Model path does not exist. Exiting ..";
      exit(1);
    }
    input_shape = std::vector<int64_t>{1, conf.input_channels(),
                                       conf.input_width(), conf.input_height()};

    img_size = cv::Size(config.input_width(), config.input_height());
  }

  // Ort::Value read_input_image(std::vector<float> &vec) {
  //   Ort::Value inp_tens =
  //       Ort::Value::CreateTensor(memory_info, vec.data(), vec.size(),
  //                                input_shape.data(), input_shape.size());
  //   return inp_tens;
  // }

  void set_input_image(cv::Mat &img) {

    cv::Mat img_mat = preprocess_img(img, img_size, 0);

    assert(img_mat.type() == CV_32FC3 && "Image type must be CV_32FC3");
    assert(img_mat.isContinuous() && "Image must be in contiguous memory");
    // create 4d array from image and save into
    cv::dnn::blobFromImage(img_mat, input_image, 1.0, img_size, cv::Scalar(),
                           false, false);
  }

  std::vector<Ort::Value> detect() {
    // TODO( andy ) find out how to define these globally
    const char *input_names[] = {config.input_names().data()};
    const char *output_names[] = {config.output_names().data()};
    Ort::RunOptions run_options;
    Ort::Value input_tens = Ort::Value::CreateTensor<float>(
        memory_info, input_image.ptr<float>(), input_image.total(),
        input_shape.data(), input_shape.size());
    auto out_tens =
        session.Run(run_options, input_names, &input_tens, 1, output_names, 1);
    return out_tens;
  }

  void get_input_output_names() {
    Ort::AllocatorWithDefaultOptions allocator;
    size_t input_count = session.GetInputCount();
    size_t output_count = session.GetOutputCount();
    auto inp_type = session.GetInputTypeInfo(0);

    for (size_t i = 0; i < input_count; ++i) {
      auto input_name = session.GetInputNameAllocated(i, allocator);
      std::cout << "Input name " << i << ": " << input_name.get() << std::endl;
    }

    for (size_t i = 0; i < output_count; ++i) {
      auto output_name = session.GetOutputNameAllocated(i, allocator);
      std::cout << "Output name " << i << ": " << output_name.get()
                << std::endl;
    }
    std::cout << inp_type << std::endl;
  }

  void get_output_type_info() {
    auto ti = session.GetOutputTypeInfo(0);
    std::cout << "out type info get const: " << ti.GetConst() << std::endl;
    std::cout << "out type info get onnx type: " << ti.GetONNXType()
              << std::endl;
    std::cout << "out tensor shape: (";
    for (auto e : ti.GetTensorTypeAndShapeInfo().GetShape()) {
      std::cout << e << ", ";
    }
    std::cout << ")" << std::endl;
    std::cout << "Type: " << ti.GetTensorTypeAndShapeInfo().GetElementType()
              << std::endl;
  }

  std::vector<Ort::Value> run(Ort::Value &inp_tens) {
    // TODO(andreas) find out how to define these globally
    const char *input_names[] = {config.input_names().data()};
    const char *output_names[] = {config.output_names().data()};
    Ort::RunOptions run_options;
    auto out_tens =
        session.Run(run_options, input_names, &inp_tens, 1, output_names, 1);
    // delete *input_names;
    // delete *output_names;
    return out_tens;
  }

  std::vector<ObbDetection> postprocess(std::vector<Ort::Value> &output,
                                        float thresh = 0.6) {
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
};

#endif // SRC_ONNXINFERLIB_INFERENCE_H
