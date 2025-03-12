
#include <iostream>
#include <array>
#include <cstdint>
#include<onnxruntime_cxx_api.h>


template <typename T>
static void softmax(T& input) {
  float rowmax = *std::max_element(input.begin(), input.end());
  std::vector<float> y(input.size());
  float sum = 0.0f;
  for (size_t i = 0; i != input.size(); ++i) {
    sum += y[i] = std::exp(input[i] - rowmax);
  }
  for (size_t i = 0; i != input.size(); ++i) {
    input[i] = y[i] / sum;
  }
}



struct ResNetSession {
    // sample onnx inference session
    // member definition

    static const int width_ = 500;
    static const int height_ = 334;
    std::array<float, width_ * height_> input_image_{};
    std::array<float, 10> results_{};
    int64_t result_{0};

    ResNetSession() {

    auto memory_info = Ort::MemoryInfo::CreateCpu(OrtDeviceAllocator, OrtMemTypeCPU);
    input_tensor_ = Ort::Value::CreateTensor<float>(memory_info, input_image_.data(), input_image_.size(),
                                                    input_shape_.data(), input_shape_.size());
    output_tensor_ = Ort::Value::CreateTensor<float>(memory_info, results_.data(), results_.size(),
                                                     output_shape_.data(), output_shape_.size());
    };



  void print_info(){
        std::cout << "ResNet Session here\n";
        std::cout << width_ << ", " << height_ << std::endl;


    };

  std::ptrdiff_t Run() {
    const char* input_names[] = {"Input3"};
    const char* output_names[] = {"Plus214_Output_0"};

    Ort::RunOptions run_options;
    session_.Run(run_options, input_names, &input_tensor_, 1, output_names, &output_tensor_, 1);
    softmax(results_);
    result_ = std::distance(results_.begin(), std::max_element(results_.begin(), results_.end()));
    return result_;
  }

private:
  Ort::Env env;
  Ort::Session session_{env, "../resnet.onnx", Ort::SessionOptions{nullptr}};

  Ort::Value input_tensor_{nullptr};
  std::array<int64_t, 4> input_shape_{1, 1, width_, height_};

  Ort::Value output_tensor_{nullptr};
  std::array<int64_t, 2> output_shape_{1, 10};
};

