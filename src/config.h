#ifndef SRC_CONFIG_H
#define SRC_CONFIG_H

#include <cstdint>
#include <string_view>
const unsigned int listen_port = 9002;
constexpr uint16_t heartbeat_interval = 5;
constexpr std::string_view model_path =
    "/Users/anlhr/Projects/onnx_websockets/models/yolo11x_obb.onnx";
constexpr std::string_view broker_addr = "127.0.0.1";
constexpr int broker_listen_port = 5672;

#endif // SRC_CONFIG_H
