#ifndef SRC_ONNXINFERLIB_INFERENCE_H
#define SRC_ONNXINFERLIB_INFERENCE_H
#include <filesystem>
#include <functional>
#include <iostream>
#include <numeric>
#include <onnxruntime_cxx_api.h>
#include <opencv2/core.hpp>
#include <opencv2/opencv.hpp>
#include <string>
#include <string_view>

struct CustOnnxConfig {
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
  static constexpr int input_width() { return _input_width; };
  static constexpr int input_height() { return _input_height; };
  static constexpr int input_channels() { return _input_channels; };
  static constexpr int output_classes() { return _output_classes; };
  // static constexpr std::string_view input_names() { return _input_name; };
  // static constexpr std::string_view output_names() { return _output_name; };
  static const char *input_names() { return _input_name.data(); };
  static const char *output_names() { return _output_name.data(); };
  static const char *model_name() { return _model_name.data(); };
  static const char *device() { return _device.data(); };
};

Ort::MemoryInfo get_mem_info(std::string memtype);

struct Yolov11Session {
  /*
   * The output of the yolov11 model is a tensor of size (batch_size, 84, 8400)
   * where 84 = 4 coordiates + 80 classes
   * 8400 = aggregated predictions from three detection heads
   */
private:
  Ort::Env env;
  Ort::Session session{nullptr};
  std::array<int64_t, 3> output_shape; // {1, 5, 8400};

public:
  // Yolov11 inference session
  static const int width = CustOnnxConfig::input_width();
  static const int height =
      CustOnnxConfig::input_height(); // get this dynamically
  static const int channels = CustOnnxConfig::input_channels();
  static constexpr std::array<int64_t, 4> input_shape{1, 3, width, height};

  cv::Mat input_image{};
  // std::array<float, width * height * channels> input_image{};
  Ort::MemoryInfo memory_info = get_mem_info(CustOnnxConfig::device());
  std::array<float, CustOnnxConfig::output_classes()> results{};
  Ort::Value input_tensor{nullptr};
  Ort::Value output_tensor{nullptr};
  std::string_view input_names;
  std::string_view output_names;
  int64_t result{0};

  Yolov11Session(std::filesystem::path model_path) {
    input_names = CustOnnxConfig::input_names();
    output_names = CustOnnxConfig::output_names();
    // make sure file path to model exists
    if (std::filesystem::exists(model_path)) {
      session =
          Ort::Session(env, model_path.c_str(), Ort::SessionOptions{nullptr});
    } else {
      std::cerr << "Model path does not exist. Exiting ..";
      exit(1);
    };

    // output_tensor = Ort::Value::CreateTensor(memory_info, results.data(),
    // results.size(),
    //                                          output_shape.data(),
    //                                          output_shape.size());
  }

  Ort::Value read_input_image(std::vector<float> &vec) {
    Ort::Value inp_tens =
        Ort::Value::CreateTensor(memory_info, vec.data(), vec.size(),
                                 input_shape.data(), input_shape.size());
    return inp_tens;
  }

  void set_input_image(cv::Mat &img) {
    assert(img.type() == CV_32FC3 && "Image type must be CV_32FC3");
    assert(img.isContinuous() && "Image must be in contiguous memory");
    cv::dnn::blobFromImage(img, input_image, 1.0, cv::Size(640, 640),
                           cv::Scalar(), false, false);
  };

  std::vector<Ort::Value> detect() {
    // TODO find out how to define these globally
    const char *input_names[] = {CustOnnxConfig::input_names()};
    const char *output_names[] = {CustOnnxConfig::output_names()};
    Ort::RunOptions run_options;
    Ort::Value input_tens = Ort::Value::CreateTensor<float>(
        memory_info, input_image.ptr<float>(), input_image.total(),
        input_shape.data(), input_shape.size());
    auto out_tens =
        session.Run(run_options, input_names, &input_tens, 1, output_names, 1);
    return out_tens;
  };

  // Ort::Value read_input(const cv::Mat &img) {
  // //     // Allocate a buffer that ONNX Runtime will manage
  //
  //     Ort::MemoryInfo mem_info =
  //     Ort::MemoryInfo::CreateCpu(OrtDeviceAllocator, OrtMemTypeDefault);
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

  void get_input_output_names() {
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
      std::cout << "Output name " << i << ": " << output_name.get()
                << std::endl;
    };
    std::cout << inp_type << std::endl;
  };

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
  };

  std::vector<Ort::Value> run(Ort::Value &inp_tens) {
    // TODO find out how to define these globally
    const char *input_names[] = {CustOnnxConfig::input_names()};
    const char *output_names[] = {CustOnnxConfig::output_names()};
    Ort::RunOptions run_options;
    auto out_tens =
        session.Run(run_options, input_names, &inp_tens, 1, output_names, 1);
    // delete *input_names;
    // delete *output_names;
    return out_tens;
  };

  ptrdiff_t postprocess(std::vector<Ort::Value> &output) {

    // Get the shape & data type of the tensor
    auto shape = output[0].GetTensorTypeAndShapeInfo().GetShape();
    ONNXTensorElementDataType type =
        output[0].GetTensorTypeAndShapeInfo().GetElementType();

    // std::cout << "shape: ( ";
    // for (int64_t s: shape){
    //     std::cout << s << ", ";
    // };
    // std::cout << ")" << std::endl;
    // std::cout << "Type: " << type << std::endl;

    const void *raw_data = output[0].GetTensorData<void>();
    if (type == ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT) {
      const float *float_data = static_cast<const float *>(raw_data);
      size_t num_elements = std::accumulate(shape.begin(), shape.end(), 1,
                                            std::multiplies<size_t>());

      // std::cout << "Num elements: " << num_elements;
      // int dim1 = shape.at(1);
      // int dim2 = shape.at(2);

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
      // int class_id = std::max_element(class_scores, class_scores + 84) -
      // class_scores;

      // std::vector<float> scores {8400};

      // for(int c = 0; c < 300; ++c){
      //     std::vector<float> det(transposed.begin()+c*dim2,
      //     transposed.begin()+c*dim2+dim2); int class_id =
      //     *std::max_element(det.begin()+4, det.end()); float class_score =
      //     det.at(class_id + 4); scores.push_back(class_score); std::cout <<
      //     "detection: cx= " << det.at(0) << " cy=" << det.at(1) << " w=" <<
      //     det.at(2) << " h=" << det.at(3); std::cout << " total_score=" <<
      //     class_score << " class_id=" << class_id; std::cout << std::endl;
      //     // std::cout << "c = " << c << std::endl;
      //     if (class_score > 0.1) {
      //         std::cout << "detection: cx= " << det.at(0) << " cy=" <<
      //         det.at(1) << " w=" << det.at(2) << " h=" << det.at(3);
      //         std::cout << " total_score=" << class_score << " class_id=" <<
      //         class_id; std::cout << std::endl;
      //     };
      // }
      // std::cout << "Max score: "<<
      // scores.at(*std::max_element(scores.begin(), scores.end()));
      std::cout << "Confidence: " << float_data[4] << "Class: " << float_data[5]
                << std::endl;
      return float_data[5];
    } else {
      return -1;
    };
  };
};

#endif // SRC_ONNXINFERLIB_INFERENCE_H
