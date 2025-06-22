#ifndef SRC_ONNXINFERLIB_ONNX_CONFIG_H
#define SRC_ONNXINFERLIB_ONNX_CONFIG_H

#include <memory>
#include <string>
#include <string_view>

class OnnxConfiguration {

private:
  const int _input_width;
  const int _input_height;
  const int _input_channels;
  const int _output_classes;
  const int _num_inputs;
  const std::string_view _input_name;
  const std::string_view _output_name;
  const std::string_view _model_name;
  const std::string_view _device;

  static std::unique_ptr<OnnxConfiguration> _config;

  OnnxConfiguration(int inp_width, int inp_height, int inp_channels,
                    int output_classes, int num_inputs,
                    const std::string &_input_name,
                    const std::string &_output_name,
                    const std::string &_model_name, const std::string &_device);

  OnnxConfiguration(const OnnxConfiguration &) = delete;
  OnnxConfiguration &operator=(const OnnxConfiguration &) = delete;

public:
  static OnnxConfiguration &
  Config(int inp_width, int inp_height, int inp_channels, int output_classes,
         int num_inputs, const std::string &_input_name,
         const std::string &_output_name, const std::string &_model_name,
         const std::string &_device);

  int input_width() const;
  int input_height() const;
  int input_channels() const;
  int output_classes() const;
  std::string_view input_names() const;
  std::string_view output_names() const;
  std::string_view model_name() const;
  std::string_view device() const;
};
#endif // SRC_ONNXINFERLIB_ONNX_CONFIG_H
