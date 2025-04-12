
#include <algorithm>
#include <cassert>
#include <filesystem>
#include <cstddef>
#include <functional>
#include <iostream>
#include <array>
#include <cstdint>
#include <iterator>
#include <memory>
#include <numeric>
#include <onnxruntime_cxx_api.h>
#include <opencv2/core.hpp>
#include <opencv2/core/hal/interface.h>
#include <opencv2/core/mat.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <string>
#include <string_view>
#include "debug_utils.h"

struct DetectionCandidate {
    float cx;
    float cy;
    float w;
    float h;
    std::vector<float> probs;

    DetectionCandidate(float cx, float cy, float w, float h, std::vector<float> probs):
    cx(cx), cy(cy), w(w), h(h), probs(probs)
    {;
    };
};



struct CustOnnxConfig{
    /*
     * Keep all configs regarding model here.
     * This has to be adapted.
     * */
private:
    static constexpr int _input_width = 640;
    static constexpr int _input_height = 640;
    static constexpr int _input_channels = 3;
    static constexpr int _output_classes = 80;
    static constexpr int _num_inputs = 1;

    static constexpr std::string_view _input_name = "images";
    static constexpr std::string_view _output_name = "output0";
    static constexpr std::string_view _model_name = "../resnet_101.onnx";
    static constexpr std::string_view _device = "cpu";


    // const std::array<std::string, _num_inputs> _input_names;

public:
    static constexpr int input_width(){ return _input_width; };
    static constexpr int input_height(){ return _input_height; };
    static constexpr int input_channels(){ return _input_channels; };
    static constexpr int output_classes(){ return _output_classes; };
    // static constexpr std::string_view input_names() { return _input_name; };
    // static constexpr std::string_view output_names() { return _output_name; };
    static const char* input_names() { return _input_name.data(); };
    static const char* output_names() { return _output_name.data(); };
    static const char* model_name() { return _model_name.data(); };
    static const char* device() {return _device.data();};
};



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


Ort::MemoryInfo get_mem_info(std::string memtype){
    if (memtype == "cpu"){
        return Ort::MemoryInfo::CreateCpu(OrtDeviceAllocator, OrtMemTypeCPU);
    }
    else {
        throw "GPU memory info not implemented yet.";
    };
}

struct Yolov11Session {
    /* 
     * The output of the yolov11 model is a tensor of size (batch_size, 84, 8400)
     * where 84 = 4 coordiates + 80 classes
     * 8400 = aggregated predictions from three detection heads
      */
private:
    Ort::Env env;
    Ort::Session session {nullptr};
    std::array<int64_t, 3> output_shape;  // {1, 5, 8400};

public:
    // Yolov11 inference session
    static const int width= CustOnnxConfig::input_width();
    static const int height = CustOnnxConfig::input_height();
    static const int channels = CustOnnxConfig::input_channels();
    static constexpr std::array<int64_t, 4> input_shape {1, 3, width, height};

    cv::Mat input_image{};
    // std::array<float, width * height * channels> input_image{};
    Ort::MemoryInfo memory_info = get_mem_info(CustOnnxConfig::device());
    std::array<float, CustOnnxConfig::output_classes()> results{};
    Ort::Value input_tensor {nullptr};
    Ort::Value output_tensor {nullptr};
    std::string_view input_names;
    std::string_view output_names;
    int64_t result{0};

    Yolov11Session(std::filesystem::path model_path)
    {
        input_names = CustOnnxConfig::input_names();
        output_names = CustOnnxConfig::output_names();
        // make sure file path to model exists
        if (std::filesystem::exists(model_path)){
            session = Ort::Session(env, model_path.c_str(), Ort::SessionOptions{nullptr});
        }
        else {
            std::cerr << "Model path does not exist. Exiting ..";
            exit(1);
        };

        // output_tensor = Ort::Value::CreateTensor(memory_info, results.data(), results.size(),
        //                                          output_shape.data(), output_shape.size());
    }

    Ort::Value read_input_image(std::vector<float> &vec){
        Ort::Value inp_tens = Ort::Value::CreateTensor(
            memory_info,
            vec.data(),
            vec.size(),
            input_shape.data(),
            input_shape.size()
        );
        return inp_tens;
    }
    // void read_input(cv::Mat &img){
    //
    //     float *buffer = img.ptr<float>();
    //     // TODO check that input shape == image shape
    //     input_tensor = Ort::Value::CreateTensor<float>(
    //         memory_info,
    //         buffer,
    //         img.total() * img.channels(),
    //         input_shape.data(),
    //         input_shape.size()
    //     );
    // };

    void set_input_image(cv::Mat &img){
        cv::dnn::blobFromImage(img, input_image, 1.0, cv::Size(640, 640), cv::Scalar(), false, false);
    };

    // Ort::Value read_input(const cv::Mat &img) {  
    // //     // Allocate a buffer that ONNX Runtime will manage  
    //
    //     Ort::MemoryInfo mem_info = Ort::MemoryInfo::CreateCpu(OrtDeviceAllocator, OrtMemTypeDefault);
    //
    //     Ort::Value in = Ort::Value::CreateTensor<float>(
    //         memory_info,
    //         img.ptr<float>(),
    //         img.total(),
    //         input_shape.data(),
    //         input_shape.size()
    //     );
    //
    //
    //     float* tensor_data = in.GetTensorMutableData<float>();
    //     return in;
    // };  


    void get_input_output_names(){
        Ort::AllocatorWithDefaultOptions allocator;
        size_t input_count = session.GetInputCount();
        size_t output_count = session.GetOutputCount();
        auto inp_type = session.GetInputTypeInfo(0);

        for (size_t i = 0; i < input_count; ++i) {
            auto input_name = session.GetInputNameAllocated(i, allocator);
            std::cout << "Input name " << i << ": " << input_name.get() << std::endl;
        };

        for (size_t i = 0; i < output_count; ++i) {
            auto output_name = session.GetOutputNameAllocated(i, allocator);
            std::cout << "Output name " << i << ": " << output_name.get() << std::endl;
        };
        std::cout << inp_type << std::endl;
    };

    void get_output_type_info(){
        auto ti = session.GetOutputTypeInfo(0);
        std::cout << "out type info get const: "<< ti.GetConst() << std::endl;
        std::cout << "out type info get onnx type: "<< ti.GetONNXType() << std::endl;
        std::cout << "out tensor shape: (";
        for (auto e: ti.GetTensorTypeAndShapeInfo().GetShape()){
            std::cout << e << ", ";
        }
        std::cout << ")" << std::endl;
        std::cout << "type: " << ti.GetTensorTypeAndShapeInfo().GetElementType() << std::endl;
    };

    std::vector<Ort::Value> run(Ort::Value &inp_tens){
        // TODO find out how to define these globally 
        const char* input_names[] = {CustOnnxConfig::input_names()};
        const char* output_names[] = {CustOnnxConfig::output_names()};
        Ort::RunOptions run_options;
        auto out_tens = session.Run(run_options, input_names, &inp_tens, 1, output_names, 1);
        // delete *input_names;
        // delete *output_names;
        return out_tens;
    };


ptrdiff_t postprocess(std::vector<Ort::Value>& output){

        // Get the shape & data type of the tensor
        auto shape = output[0].GetTensorTypeAndShapeInfo().GetShape();
        ONNXTensorElementDataType type = output[0].GetTensorTypeAndShapeInfo().GetElementType();

        std::cout << "shape: ( ";
        for (int64_t s: shape){
            std::cout << s << ", ";
        };
        std::cout << ")" << std::endl;
        std::cout << "Type: " << type << std::endl;

        const void* raw_data = output[0].GetTensorData<void>();
        if (type == ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT) {
            const float* float_data = static_cast<const float*>(raw_data);
            size_t num_elements = std::accumulate(shape.begin(), shape.end(), 1, std::multiplies<size_t>());

            std::cout << "Num elements: " << num_elements;
            int dim1 = shape.at(1);
            int dim2 = shape.at(2);
            
            // std::vector<float> transposed(num_elements);
            // for (int i = 0; i < dim1; ++i) {
            //     for (int j = 0; j < dim2; ++j) {
            //         transposed[i * dim2 + j] = 
            //             float_data[j * dim1 + i];
            //     }
            // }
            // 
            // for (auto k: transposed){
            //
            // }


            // for (int i=0; i < 8400 ; ++i){
            //     float cx = float_data[0 * 8400 + i];
            //     float cy = float_data[1 * 8400 + i]; 
            //     float w = float_data[2 * 8400 + i];
            //     float h = float_data[3 * 8400 + i]; 
            //
            //     std::vector<float> probs {};
            //     for (int cls = 0; cls < 80; ++cls){
            //         probs.push_back(float_data[(4 + cls) * 8400 + i]);
            //     }
            //
            //     auto det = DetectionCandidate(
            //          cx,
            //          cy,
            //          w,
            //          h,
            //          std::move(probs)
            //     );
                
                // const float* anchor_data = &float_data[84];
                // float cx = anchor_data[0]
                // float cy = anchor_data[1];
                // float w = anchor_data[2];
                // float h = anchor_data[3];                
                // float obj_score = anchor_data[4];  
                // const float* class_scores = &anchor_data[4];
                // int class_id = std::max_element(class_scores, class_scores + 84) - class_scores;
            

            // std::vector<float> scores {8400};

            // for(int c = 0; c < 300; ++c){
            //     std::vector<float> det(transposed.begin()+c*dim2, transposed.begin()+c*dim2+dim2); 
            //     int class_id = *std::max_element(det.begin()+4, det.end());
            //     float class_score = det.at(class_id + 4);
            //     scores.push_back(class_score);
            //     std::cout << "detection: cx= " << det.at(0) << " cy=" << det.at(1) << " w=" << det.at(2) << " h=" << det.at(3);
            //     std::cout << " total_score=" << class_score << " class_id=" << class_id;
            //     std::cout << std::endl;
            //     // std::cout << "c = " << c << std::endl;
            //     if (class_score > 0.1) {
            //         std::cout << "detection: cx= " << det.at(0) << " cy=" << det.at(1) << " w=" << det.at(2) << " h=" << det.at(3);
            //         std::cout << " total_score=" << class_score << " class_id=" << class_id;
            //         std::cout << std::endl;
            //     };
            // }
            // std::cout << "Max score: "<< scores.at(*std::max_element(scores.begin(), scores.end()));
            std::cout << "Confidence: " << float_data[4] << "Class: " << float_data[5] << std::endl;
            return float_data[5];
        };
    };

// ptrdiff_t postprocess(std::vector<Ort::Value> output){
//         const float* probs = output[0].GetTensorData<float>();
//         const float* end = probs + 80;
//         const float* max_p = std::max_element(probs+1, end);
//         auto max_p_index = std::distance(probs, max_p);
//         assert(max_p_index >= 1);
//         std::cout << "Max index <=> Label: "<< max_p_index << std::endl;
//         // softmax(results_);
//         // result_ = std::distance(results_.begin(), std::max_element(results_.begin(), results_.end()));
//         // return result_;
//         delete probs;
//         return max_p_index;
//     };
};


struct ResNetSession {
    // sample onnx inference session
    // member definition
    static const int width_= CustOnnxConfig::input_width();
    static const int height_ = CustOnnxConfig::input_height();
    static const int channels_ = CustOnnxConfig::input_channels();
    std::array<float, width_ * height_ * channels_> input_image_{};
    std::array<float, CustOnnxConfig::output_classes()> results_{};
    int64_t result_{0};
    Ort::MemoryInfo memory_info;

    ResNetSession(): 
        // std::array<std::string, 1> nms {"images"};
        // CustOnnxConfig conf = CustOnnxConfig(nms);

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
    // output_tensor_ = Ort::Value::CreateTensor<float>(memory_info, results_.data(), results_.size(),
    //                                                  output_shape_.data(), output_shape_.size());
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
        cv::resize(img, resized_array, cv::Size(width_, height_));
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

        const char* input_names[] = {CustOnnxConfig::input_names()};
        const char* output_names[] = {CustOnnxConfig::output_names()};
        // const char* input_names[] = {"data"};
        //const char* output_names[] = {"resnetv18_dense0_fwd"};

        Ort::RunOptions run_options;
        session_.Run(run_options, input_names, &input_tensor_, 1, output_names, &output_tensor_, 1);
        float* probs = output_tensor_.GetTensorMutableData<float>();
        float* end = probs + 1001;
        float* max_p = std::max_element(probs+1, end);
        auto max_p_index = std::distance(probs, max_p);
        assert(max_p_index >= 1);
        std::cout << "Max index <=> Label: "<< max_p_index << std::endl;
        // softmax(results_);
        // result_ = std::distance(results_.begin(), std::max_element(results_.begin(), results_.end()));
        // return result_;
        return max_p_index;
    }

    std::ptrdiff_t detect(cv::Mat img){
        /*
         * Run inference on img
         * */
        float *buffer = img.ptr<float>();
        this->set_input_tensor(buffer, img.total() * img.channels());
        std::ptrdiff_t res = this->Run();
        return res;
    };


std::ptrdiff_t onnx_run(ResNetSession& sess, cv::Mat img){
    /*
     * Run img in sess.
     * */
    float *buffer = img.ptr<float>();
    sess.set_input_tensor(buffer, img.total() * img.channels());
    std::ptrdiff_t res = sess.Run();
    return res;
};

private:
  Ort::Env env;
  Ort::Session session_{env, CustOnnxConfig::model_name(), Ort::SessionOptions{nullptr}};

  Ort::Value input_tensor_{nullptr};
  std::array<int64_t, 4> input_shape_{1, 3, width_, height_};

  Ort::Value output_tensor_{nullptr};
  std::array<int64_t, 2> output_shape_{1, 1000};
};
