
#include "onnxruntime_c_api.h"
#include <algorithm>
#include <iostream>
#include <array>
#include <cstdint>
#include<onnxruntime_cxx_api.h>
#include <opencv2/core.hpp>
#include <opencv2/core/mat.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>



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

    static const int width_ = 224;
    static const int height_ = 224;
    static const int channels_ = 3;
    std::array<float, width_ * height_ * channels_> input_image_{};
    std::array<float, 1000> results_{};
    int64_t result_{0};
    Ort::MemoryInfo memory_info;

    ResNetSession(): 
        memory_info(Ort::MemoryInfo::CreateCpu(OrtDeviceAllocator, OrtMemTypeCPU)) 
    {
    // auto memory_info = Ort::MemoryInfo::CreateCpu(OrtDeviceAllocator, OrtMemTypeCPU);
        //
    input_tensor_ = Ort::Value::CreateTensor<float>(
            memory_info,
            input_image_.data(),
            input_image_.size(),
            input_shape_.data(),
            input_shape_.size()
        );
    output_tensor_ = Ort::Value::CreateTensor<float>(memory_info, results_.data(), results_.size(),
                                                     output_shape_.data(), output_shape_.size());
    };

    void print_info(){
        std::cout << "ResNet Session here\n";
        std::cout << width_ << ", " << height_ << std::endl;
    };

    


    void set_input_tensor(float *buffer, size_t num_elem){

        input_tensor_ = Ort::Value::CreateTensor<float>(
            memory_info,
            buffer,
            // img.total() * img.channels(),
            num_elem,
            input_shape_.data(),
            input_shape_.size()
        );

    };
    void read_input_tensor(cv::Mat &img){

        // resize 
        cv::Mat resized_array;
        cv::Mat final_img;
        cv::resize(img, resized_array, cv::Size(224, 224));
        resized_array.convertTo(final_img, CV_32F, 1.0/255.0);

        double minVal;
        double maxVal;
        cv::Point minLoc;
        cv::Point maxLoc; 
        cv::minMaxLoc(final_img, &minVal, &maxVal);
        std::cout << minVal << " " << maxVal << std::endl;

        float *buffer = final_img.ptr<float>();
        input_tensor_ = Ort::Value::CreateTensor<float>(
            memory_info,
            buffer,
            // img.total() * img.channels(),
            final_img.total() * final_img.channels(),
            input_shape_.data(),
            input_shape_.size()
        );
    };


    void getInputAndOutputNames(){

        Ort::AllocatorWithDefaultOptions allocator;
        size_t input_count = session_.GetInputCount();
        size_t output_count = session_.GetOutputCount();
        auto inp_type = session_.GetInputTypeInfo(0);

        for (size_t i = 0; i < input_count; ++i) {
            auto input_name = session_.GetInputNameAllocated(i, allocator);
            std::cout << "Input name " << i << ": " << input_name.get() << std::endl;
        };

        for (size_t i = 0; i < output_count; ++i) {
            auto output_name = session_.GetOutputNameAllocated(i, allocator);
            std::cout << "Output name " << i << ": " << output_name.get() << std::endl;
        };

        std::cout << inp_type << std::endl;
    }

    std::ptrdiff_t Run() {
        const char* input_names[] = {"data"};
        const char* output_names[] = {"resnetv18_dense0_fwd"};

        Ort::RunOptions run_options;
        session_.Run(run_options, input_names, &input_tensor_, 1, output_names, &output_tensor_, 1);

        softmax(results_);
        for (size_t i = 0; i < 10; i++){
            std::cout << results_[i] << std::endl;
        }
        result_ = std::distance(results_.begin(), std::max_element(results_.begin(), results_.end()));
        return result_;
    }

private:
  Ort::Env env;
  Ort::Session session_{env, "../resnet_101.onnx", Ort::SessionOptions{nullptr}};

  Ort::Value input_tensor_{nullptr};
  std::array<int64_t, 4> input_shape_{1, 3, width_, height_};

  Ort::Value output_tensor_{nullptr};
  std::array<int64_t, 2> output_shape_{1, 1000};
};


