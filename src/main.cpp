#include "amqpcpp/message.h"
#include "websocketpp/common/connection_hdl.hpp"
#include "websocketpp/frame.hpp"
#include <cctype>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <functional>
#include <iostream>
#include <memory>
#include <opencv2/core.hpp>
#include <opencv2/core/base.hpp>
#include <opencv2/core/hal/interface.h>
#include <opencv2/core/mat.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <ostream>
#include <string>
#include <unistd.h>
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
// custom lib imports
#include "ampq.h"
#include "ampq_socket.h"
#include "base64.h"
#include "config.h"
#include "debug_utils.h"
#include "inference.h"
#include "utils.h"

#include <amqpcpp.h>

constexpr int DEBUG = 1;
typedef websocketpp::server<websocketpp::config::asio> server;

class utility_server;
void outside_handler(utility_server &us, websocketpp::connection_hdl hdl,
                     server::message_ptr msg);
// cv::Mat preprocess_img(cv::Mat& mat, cv::Size sz, bool);

class utility_server {
public:
  utility_server() {
    // Set logging settings
    m_endpoint.set_error_channels(websocketpp::log::elevel::all);
    m_endpoint.set_access_channels(websocketpp::log::alevel::all ^
                                   websocketpp::log::alevel::frame_payload);

    // Initialize Asio
    m_endpoint.init_asio();

    /*
     * Need the std::bind here since we pass a member function  and the 'this'
     * argument is passed std::placeholders::_1 is corresponds to first arg
     * passed call, analogously for 2
     * */

    // m_endpoint.set_message_handler(std::bind(
    //     &utility_server::duplicate_handler, this,
    //     std::placeholders::_1, std::placeholders::_2
    // ));
    //

    // m_endpoint.set_message_handler(std::function(&outside_handler));
    // m_endpoint.set_message_handler(std::bind(
    //     &outside_handler, this,
    //     std::placeholders::_1, std::placeholders::_2
    // ));
  };

  void send(websocketpp::connection_hdl hdl, std::string &s,
            websocketpp::frame::opcode::value op) {
    m_endpoint.send(hdl, s, op);
  };

  void set_message_handler(
      std::function<void(websocketpp::connection_hdl, server::message_ptr msg)>
          func) {
    m_endpoint.set_message_handler(func);
  };

  void run(const unsigned int listen_port) {
    m_endpoint.listen(listen_port);
    // Queues a connection accept operation
    m_endpoint.start_accept();
    // Start the Asio io_service run loop
    m_endpoint.run();
  }

  void image_handler(websocketpp::connection_hdl hdl, server::message_ptr msg) {
    // verify that message type is binary
    m_endpoint.send(hdl, msg->get_payload(), msg->get_opcode());
  }

  void duplicate_handler(websocketpp::connection_hdl hdl,
                         server::message_ptr msg) {
    // example handler member function impl
    // write a new message
    // do something with payload - here duplication
    std::string new_msg_payload(
        msg->get_raw_payload() +
        msg->get_raw_payload()); // non const ref to payload
    m_endpoint.send(hdl, new_msg_payload, msg->get_opcode());
  }

private:
  server m_endpoint;
};

// message handler used for websocket server
// void outside_handler(utility_server &us, resnetsession &sess,
// websocketpp::connection_hdl hdl, server::message_ptr msg) {
//     // write a new message
//     std::string new_msg_payload;
//     if (msg->get_opcode() == websocketpp::frame::opcode::binary){
//         std::vector<uchar> payload_2(msg->get_payload().begin(),
//         msg->get_payload().end()); cv::Mat img = cv::imdecode(payload_2,
//         cv::IMREAD_COLOR_RGB); cv::Mat new_img = preprocess_img(img,
//         cv::Size(224,224), 1); if (new_img.empty()) {
//             throw std::runtime_error("Failed to decode image from byte
//             string");
//         }
//         auto res = sess.detect(new_img);
//         std::cout << "Result: " << res << std::endl;
//         new_msg_payload = "Bytes frame. Image has size (" +
//             std::to_string(new_img.rows) +
//             "," +
//             std::to_string(new_img.cols) +
//             ") yielding result: " +
//             std::to_string(res); // non const ref to payload
//     }
//     else {
//         new_msg_payload = "This is a text frame"; // non const ref to payload
//     };
//     us.send(hdl, new_msg_payload, websocketpp::frame::opcode::text);
// }

void yolo_handler(utility_server &us, Yolov11Session &sess,
                  websocketpp::connection_hdl hdl, server::message_ptr msg) {
  // write a new message
  std::string new_msg_payload;
  if (msg->get_opcode() == websocketpp::frame::opcode::binary) {
    std::vector<uchar> payload_2(msg->get_payload().begin(),
                                 msg->get_payload().end());
    cv::Mat img = cv::imdecode(payload_2, cv::IMREAD_COLOR_RGB);
    cv::Mat new_img = preprocess_img(img, cv::Size(640, 640), 0);
    if (new_img.empty()) {
      throw std::runtime_error("Failed to decode image from byte string");
    }
    sess.set_input_image(new_img);
    // sess.read_input_image(new_img);
    auto onnx_out_tens = sess.detect();
    auto res = sess.postprocess(onnx_out_tens);
    //     auto res = sess.detect(new_img);
    new_msg_payload =
        "Bytes frame. Image has size (" + std::to_string(new_img.rows) + "," +
        std::to_string(new_img.cols) +
        ") yielding result: " + std::to_string(res); // non const ref to payload
  } else {
    new_msg_payload = "This is a text frame"; // non const ref to payload
  }
  us.send(hdl, new_msg_payload, websocketpp::frame::opcode::text);
}

int main(int argc, char **argv) {
  // create socket underlying AMQP connection used by connection handler
  MySocket sock({});
  sock.do_connect();

  // create an instance of custom connection handler
  auto model_path = std::filesystem::path{
      "/Users/anlhr/Projects/onnx_websockets/models/yolo11x_obb.onnx"};
  MyConnectionHandler myHandler(sock, model_path);

  // create a AMQP connection object
  AMQP::Connection connection(&myHandler, AMQP::Login("guest", "guest"), "/");

  if (DEBUG) {
    std::cout << "Connection ready: " << connection.ready() << std::endl;
    std::cout << "Connection usable: " << connection.usable() << std::endl;
    std::cout << "Connection initialized: " << connection.initialized()
              << std::endl;
  }

  int poll_status = 0;
  ssize_t received = 0;
  size_t parsed_bytes = 0;
  timeval timeout;
  timeval start, end;
  timeout.tv_usec = 0;
  timeout.tv_sec = 0;
  int64 elapsed = 0;
  // basic event loop
  while (true) {
    // every iteration check if socket is readable, if so read into buffer
    // Every couple of seconds, send a heartbeat to the Rabbit-MQ broker
    if (gettimeofday(&start, nullptr) != 0) {
      std::cerr << "Cannot get current time. Exiting..." << std::endl;
      exit(-1);
    }

    // at each iteration, clear data buffer of MySocket
    received = 0;
    sock.reset_buf();

    // check if socket is readable
    poll_status = sock.readable(timeout);
    parsed_bytes = 0;

    if (poll_status == 1) {
      // socket is readable, ready to receive message
      received = sock._receive();
      if (received > 0) {
        // Pass incoming data to the AMQP connection
        while (parsed_bytes < received) {
          // parse as long as there is
          parsed_bytes = connection.parse(sock.buffer + parsed_bytes,
                                          received - parsed_bytes);
        }

      } else if (received == 0) {
        std::cout << "Server closed connection\n";
        break;
      } else {
        std::cerr << "Recv error\n";
        break;
      }
      // read
    } else if (poll_status < 1) {
      // std::cout << "Not ready to read" << std::endl;
    }
    gettimeofday(&end, nullptr);
    // check if hearbeat must be sent
    elapsed +=
        (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_usec - start.tv_usec);
    // std::cout << "Microseconds elapsed: " << elapsed << std::endl;
    if (elapsed > 3 * 1000000) {
      // std::cout << "send heartbeat \n";
      // std::cout << "Seconds elapsed: " << elapsed / 1000000 << std::endl;
      connection.heartbeat();
      elapsed = 0;
    }
  }
  return 0;
}
