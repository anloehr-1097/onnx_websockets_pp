#pragma once
#include <amqp_socket.h>
#include <amqpcpp.h>
#include <conn_handler.h>
#include <gtest/gtest.h>

#include <filesystem>
#include <string_view>
#include <system_error>

#include "../config.h"
#include "onnx_config.h"

class ConnHandlerFixture : public testing::Test {
 protected:
  static constexpr std::string_view model_path =
      "/Users/anlhr/Projects/onnx_websockets/models/yolo11x_obb.onnx";
  std::filesystem::path mpath = std::filesystem::path{model_path.data()};

  OnnxConfiguration &yolo_config = OnnxConfiguration::Config(
      640, 640, 3, 80, 1, "images", "output0", "yolov11obb", "cpu");

  std::string_view ba{backend_addr};
  MySocket sock{broker_listen_port, broker_addr.data()};
  MyConnectionHandler handler{sock, mpath, yolo_config, ba,
                              backend_listen_port};

  // MyConnectionHandler handler(sock, mpath, yolo_config, ba,
  //                             backend_listen_port);
  // //
  AMQP::Connection con =
      AMQP::Connection(&handler, AMQP::Login("guest", "guest"), "/");

  bool ba_available = sock.connect();
};
