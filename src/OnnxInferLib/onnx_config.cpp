#include "include/onnx_config.h"
#include <iostream>
#include <memory>
#include <string>

std::unique_ptr<OnnxConfiguration> OnnxConfiguration::_config{nullptr};

OnnxConfiguration::OnnxConfiguration(int inp_width, int inp_height,
                                     int inp_channels, int output_classes,
                                     int num_inputs,
                                     const std::string &input_name,
                                     const std::string &output_name,
                                     const std::string &model_name,
                                     const std::string &device)
    : _input_width(inp_width), _input_height(inp_height),
      _input_channels(inp_channels), _output_classes(output_classes),
      _num_inputs(num_inputs), _input_name(input_name),
      _output_name(output_name), _model_name(model_name), _device(device) {

  std::cout << "Creation of OnnxConfiguration.\n";
}
OnnxConfiguration &OnnxConfiguration::Config(int inp_width, int inp_height,
                                             int inp_channels,
                                             int output_classes, int num_inputs,
                                             const std::string &input_name,
                                             const std::string &output_name,
                                             const std::string &model_name,
                                             const std::string &device) {
  if (_config == nullptr) {
    OnnxConfiguration *_conf = new OnnxConfiguration(
        inp_width, inp_height, inp_channels, output_classes, num_inputs,
        input_name, output_name, model_name, device);

    _config = std::unique_ptr<OnnxConfiguration>(_conf);
  } else {
    // check if config has same values as provided
    if (inp_width == _config->_input_width &&
        inp_height == _config->_input_height &&
        inp_channels == _config->_input_channels &&
        input_name == _config->_input_name &&
        output_name == _config->_output_name &&
        model_name == _config->_model_name && device == _config->_device) {
      return *_config;
    } else {

      OnnxConfiguration *_conf = new OnnxConfiguration(
          inp_width, inp_height, inp_channels, output_classes, num_inputs,
          input_name, output_name, model_name, device);

      _config = std::unique_ptr<OnnxConfiguration>(_conf);
    }
  }

  return *_config;
}

int OnnxConfiguration::input_width() const { return _input_width; }
int OnnxConfiguration::input_height() const { return _input_height; }
int OnnxConfiguration::input_channels() const { return _input_channels; }
int OnnxConfiguration::output_classes() const { return _output_classes; }
std::string_view OnnxConfiguration::input_names() const { return _input_name; }
std::string_view OnnxConfiguration::output_names() const {
  return _output_name;
}
std::string_view OnnxConfiguration::model_name() const { return _model_name; }
std::string_view OnnxConfiguration::device() const { return _device; }
