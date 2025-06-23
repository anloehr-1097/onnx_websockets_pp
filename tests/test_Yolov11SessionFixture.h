
#include <gtest/gtest.h>

#include <filesystem>
#include <string>

#include "inference.h"

class Yolov11Fixture : public testing::Test {
 protected:
  static constexpr std::string_view model_path = "models/yolo11x_obb.onnx";
  std::filesystem::path mpath = std::filesystem::path{model_path.data()};

  std::string input_names{"images"};
  std::string output_names{"output0"};
  std::string model_name{"output0"};
  std::string device{"cpu"};

  OnnxConfiguration &yolo_config = OnnxConfiguration::Config(
      640, 640, 3, 80, 1, input_names, output_names, model_name, device);

  Yolov11Session onnx_sess = Yolov11Session(model_path, yolo_config);
};
