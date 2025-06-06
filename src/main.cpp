#include "amqpcpp/message.h"
#include "websocketpp/common/connection_hdl.hpp"
#include "websocketpp/frame.hpp"
#include <cctype>
#include <cstddef>
#include <cstring>
#include <filesystem>
#include <functional>
#include <iostream>
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
  MySocket sock({});
  sock.do_connect();

  int poll_status = 0;
  ssize_t received = 0;
  // create an instance of your own connection handler
  MyConnectionHandler myHandler(sock);
  // create a AMQP connection object
  AMQP::Connection connection(&myHandler, AMQP::Login("guest", "guest"), "/");

  // connection.parse(sock.buffer, sock._receive());

  // AMQP::Channel channel(&connection);

  // // use the channel object to call the AMQP method you like
  // channel.declareExchange("my-exchange", AMQP::fanout);
  // // channel.declareQueue("celery", );
  // channel.bindQueue("my-exchange", "celery", "celery");

  std::cout << "Connection ready: " << connection.ready() << std::endl;
  std::cout << "Connection usable: " << connection.usable() << std::endl;
  std::cout << "Connection initialized: " << connection.initialized()
            << std::endl;

  timeval timeout;
  timeout.tv_usec = 0;
  timeout.tv_sec = 0;
  struct timeval start, end;

  int64 elapsed = 0;
  // event loop
  while (true) {
    gettimeofday(&start, nullptr);
    received = 0;
    sock.reset_buf();
    poll_status = sock.readable(timeout);
    size_t parsed_bytes = 0;

    if (poll_status == 1) {
      // socket readable
      received = sock._receive();
      // sock.print_buf(received);
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
      // std::cout << "Poll status: " << poll_status << std::endl;
    } else {
    }
    gettimeofday(&end, nullptr);
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

  // myHandler.channel->consume("celery")
  //     .onSuccess([](const std::string &tag) {
  //       std::cout << "Successful consumption yielding tag " << tag <<
  //       std::endl;
  //     })
  //     .onCancelled([](const std::string &tag) {
  //       std::cout << "Consumer tag " << tag << " was cancelled." <<
  //       std::endl;
  //     })
  //     .onReceived([&myHandler](const AMQP::Message &message,
  //                              uint64_t deliveryTag, bool redelivered) {
  //       std::cout << "received " << deliveryTag << std::endl;
  //
  //       // we remove the queue -- to see if this indeed causes the
  //       onCancelled
  //       // method to be called
  //       if (deliveryTag > 3)
  //         myHandler.channel->removeQueue("celery");
  //
  //       // ack the message
  //       myHandler.channel->ack(deliveryTag);
  //     });

  return 0;
}
// if (argc == 3 && strcmp(argv[1], "--test-mode") == 0) {
//   utility_server s;
//   auto fp = std::filesystem::path{
//       "/Users/anlhr/Projects/onnx_websockets/models/yolo11x_obb.onnx"};
//   auto onnx_sess = Yolov11Session(fp);
//   s.set_message_handler([&s, &onnx_sess](websocketpp::connection_hdl
//   hdl,
//                                          server::message_ptr msg) {
//     yolo_handler(s, onnx_sess, hdl, msg);
//   });
//
//   std::istringstream iss(argv[2]);
//   int p_listen;
//
//   if (iss >> p_listen) {
//     ;
//   } else {
//     std::cerr << "Could not convert 2nd cmdline arg to int to use for
//     port.
//     "
//                  "Running on std port.";
//     exit(1);
//   }
//   std::cout << "Server ready." << std::endl;
//   s.run(p_listen);
// }
// // create websocket server & onnx session
// utility_server s;
// auto fp = std::filesystem::path{
//     "/Users/anlhr/Projects/onnx_websockets/models/yolo11x_obb.onnx"};
// auto onnx_sess = Yolov11Session(fp);
// onnx_sess.get_input_output_names();
// onnx_sess.get_output_type_info();
// // read image from disk & preprocess image
// auto img = cv::imread(
//     "/Users/anlhr/Projects/onnx_websockets/images/test_img_broccoli.jpg",
//     cv::IMREAD_COLOR_RGB);
// cv::Mat resized_img;
// img = preprocess_img(img, cv::Size(640, 640), 0);
// assert(img.type() == CV_32FC3 && "Image type must be CV_32FC3");
// assert(img.isContinuous() && "Image must be in contiguous memory");
// onnx_sess.set_input_image(img);
// auto onnx_out_tens = onnx_sess.detect();
// auto res = onnx_sess.postprocess(onnx_out_tens);
//
// // -- server related logic --
// // set message handler for server & run server
// // s.set_message_handler([&s, &onnx_sess](websocketpp::connection_hdl
// hdl,
// // server::message_ptr msg){outside_handler(s, onnx_sess,hdl, msg);});
// s.set_message_handler([&s, &onnx_sess](websocketpp::connection_hdl hdl,
//                                        server::message_ptr msg) {
//   yolo_handler(s, onnx_sess, hdl, msg);
// });
// s.run(listen_port);
