#include <algorithm>
#include <cassert>
#include <cstddef>
#include <iostream>
#include <array>
#include <cstdint>
#include <iterator>
#include <opencv2/core.hpp>
#include <opencv2/core/hal/interface.h>
#include <opencv2/core/mat.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <string>
#include "include/inference.h"


Ort::MemoryInfo get_mem_info(std::string memtype){
    if (memtype == "cpu"){
        return Ort::MemoryInfo::CreateCpu(OrtDeviceAllocator, OrtMemTypeCPU);
    }
    else {
        throw "GPU memory info not implemented yet.";
    };
}



// struct ResNetSession {
//     // sample onnx inference session
//     // member definition
//     static const int width_= CustOnnxConfig::input_width();
//     static const int height_ = CustOnnxConfig::input_height();
//     static const int channels_ = CustOnnxConfig::input_channels();
//     std::array<float, width_ * height_ * channels_> input_image_{};
//     std::array<float, CustOnnxConfig::output_classes()> results_{};
//     int64_t result_{0};
//     Ort::MemoryInfo memory_info;
//
//     ResNetSession(): 
//         // std::array<std::string, 1> nms {"images"};
//         // CustOnnxConfig conf = CustOnnxConfig(nms);
//
//         memory_info(Ort::MemoryInfo::CreateCpu(OrtDeviceAllocator, OrtMemTypeCPU)) 
//     {
//     // auto memory_info = Ort::MemoryInfo::CreateCpu(OrtDeviceAllocator, OrtMemTypeCPU);
//         //
//     input_tensor_ = Ort::Value::CreateTensor<float>(
//             memory_info,
//             input_image_.data(),
//             input_image_.size(),
//             input_shape_.data(),
//             input_shape_.size()
//         );
//     // output_tensor_ = Ort::Value::CreateTensor<float>(memory_info, results_.data(), results_.size(),
//     //                                                  output_shape_.data(), output_shape_.size());
//     };
//
//     void print_info(){
//         std::cout << "ResNet Session here\n";
//         std::cout << width_ << ", " << height_ << std::endl;
//     };
//
//     void set_input_tensor(float *buffer, size_t num_elem){
//         input_tensor_ = Ort::Value::CreateTensor<float>(
//             memory_info,
//             buffer,
//             // img.total() * img.channels(),
//             num_elem,
//             input_shape_.data(),
//             input_shape_.size()
//         );
//     };
//
//     void read_input_tensor(cv::Mat &img){
//
//         // resize 
//         cv::Mat resized_array;
//         cv::Mat final_img;
//         cv::resize(img, resized_array, cv::Size(width_, height_));
//         resized_array.convertTo(final_img, CV_32F, 1.0/255.0);
//
//         double minVal;
//         double maxVal;
//         cv::Point minLoc;
//         cv::Point maxLoc; 
//         cv::minMaxLoc(final_img, &minVal, &maxVal);
//         std::cout << minVal << " " << maxVal << std::endl;
//
//         float *buffer = final_img.ptr<float>();
//         input_tensor_ = Ort::Value::CreateTensor<float>(
//             memory_info,
//             buffer,
//             // img.total() * img.channels(),
//             final_img.total() * final_img.channels(),
//             input_shape_.data(),
//             input_shape_.size()
//         );
//     };
//
//
//     void getInputAndOutputNames(){
//
//         Ort::AllocatorWithDefaultOptions allocator;
//         size_t input_count = session_.GetInputCount();
//         size_t output_count = session_.GetOutputCount();
//         auto inp_type = session_.GetInputTypeInfo(0);
//
//         for (size_t i = 0; i < input_count; ++i) {
//             auto input_name = session_.GetInputNameAllocated(i, allocator);
//             std::cout << "Input name " << i << ": " << input_name.get() << std::endl;
//         };
//
//         for (size_t i = 0; i < output_count; ++i) {
//             auto output_name = session_.GetOutputNameAllocated(i, allocator);
//             std::cout << "Output name " << i << ": " << output_name.get() << std::endl;
//         };
//
//         std::cout << inp_type << std::endl;
//     }
//
//     std::ptrdiff_t Run() {
//
//         const char* input_names[] = {CustOnnxConfig::input_names()};
//         const char* output_names[] = {CustOnnxConfig::output_names()};
//         // const char* input_names[] = {"data"};
//         //const char* output_names[] = {"resnetv18_dense0_fwd"};
//
//         Ort::RunOptions run_options;
//         session_.Run(run_options, input_names, &input_tensor_, 1, output_names, &output_tensor_, 1);
//         float* probs = output_tensor_.GetTensorMutableData<float>();
//         float* end = probs + 1001;
//         float* max_p = std::max_element(probs+1, end);
//         auto max_p_index = std::distance(probs, max_p);
//         assert(max_p_index >= 1);
//         std::cout << "Max index <=> Label: "<< max_p_index << std::endl;
//         // softmax(results_);
//         // result_ = std::distance(results_.begin(), std::max_element(results_.begin(), results_.end()));
//         // return result_;
//         return max_p_index;
//     }
//
//     std::ptrdiff_t detect(cv::Mat img){
//         /*
//          * Run inference on img
//          * */
//         float *buffer = img.ptr<float>();
//         this->set_input_tensor(buffer, img.total() * img.channels());
//         std::ptrdiff_t res = this->Run();
//         return res;
//     };
//
//
// std::ptrdiff_t onnx_run(ResNetSession& sess, cv::Mat img){
//     /*
//      * Run img in sess.
//      * */
//     float *buffer = img.ptr<float>();
//     sess.set_input_tensor(buffer, img.total() * img.channels());
//     std::ptrdiff_t res = sess.Run();
//     return res;
// };
//
// private:
//   Ort::Env env;
//   Ort::Session session_{env, CustOnnxConfig::model_name(), Ort::SessionOptions{nullptr}};
//
//   Ort::Value input_tensor_{nullptr};
//   std::array<int64_t, 4> input_shape_{1, 3, width_, height_};
//
//   Ort::Value output_tensor_{nullptr};
//   std::array<int64_t, 2> output_shape_{1, 1000};
// };
