#ifndef SRC_CONFIG_H
#define SRC_CONFIG_H

#include <cstdint>
#include <string_view>
const unsigned int listen_port = 9002;
constexpr uint16_t heartbeat_interval = 5;
constexpr std::string_view model_path = "models/yolo11x_obb.onnx";
constexpr std::string_view broker_addr = "127.0.0.1";
constexpr int broker_listen_port = 5672;
constexpr std::string_view backend_addr = "127.0.0.1";
constexpr int backend_listen_port = 6379;

#endif  // SRC_CONFIG_H
