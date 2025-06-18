#pragma once
#include "../config.h"
#include "conn_handler.h"
#include <amqp_socket.h>
#include <filesystem>
#include <gtest/gtest.h>
#include <string_view>
#include <system_error>

class ConnHandlerFixture : public testing::Test {

protected:
  static constexpr std::string_view model_path =
      "/Users/anlhr/Projects/onnx_websockets/models/yolo11x_obb.onnx";
  std::filesystem::path mpath = std::filesystem::path{model_path.data()};
  std::string_view ba{backend_addr};
  MySocket sock{broker_listen_port, broker_addr.data()};
  MyConnectionHandler handler{sock, mpath, ba, backend_listen_port};
  bool ba_available = sock.do_connect();
};
