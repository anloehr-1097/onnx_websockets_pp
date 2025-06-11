#include "callbacks.h"
#include "json_utils.h"
#include "tasks.h"
#include <cstdint>
#include <fstream>
#include <ios>
#include <iostream>
#include <memory>
#include <nlohmann/json.hpp>
#include <opencv2/core.hpp>
#include <string>
#include <string_view>
#include <variant>

using json = nlohmann::json;

void save_json(json json, std::string fname = "json_out.json") {
  std::ofstream fs(fname, std::ios_base::out);
  fs << json;
  fs.close();
}

void onDataCb(std::string &buf, const char *data, int64_t len) {

  std::cout << "onData called\n";
  std::cout << "First 100 bytes of data: " << std::string(data, 100) + "..."
            << std::endl;
  buf.append(data);
  // std::cout << "Buffer: " << buf << std::endl;
}

void onSuccessCb(const std::string &tag) {
  std::cout << "onSuccess called with tag " << tag << "\n";
}

void onCompleteCb(int64_t lg, bool d) {
  std::cout << "onComplete called\n";
  std::cout << "Long: " << lg << " & bool: " << d << std::endl;
}
void onReceivedCb(std::shared_ptr<AMQP::Channel> ch,
                  const AMQP::Message &message, uint64_t deliveryTag,
                  bool redelivered) {
  // std::string_view s(message.body(), 100);
  // std::cout << "Message body first 100 bytes: " << s << ".\n";
  // std::cout << "Message content type: " << message.contentType() << ".\n";
  ch->ack(deliveryTag);
  std::cout << "Acknowledged message with tag " << deliveryTag << std::endl;
  std::string_view rk{message.routingkey()};
  std::cout << "Routing key: " << rk << std::endl;
  Task task;
  if (rk == "celery") {
    std::cout << "Celery tasks\n";
    task = HelloTask;
  } else if (rk == "yolo prediction") {
    std::cout << "Inference tasks\n";
    task = PredictionTask;
  } else {
    std::cout << "Unknown task" << std::endl;
    task = UnknownTask;
  }

  // parse json based on value of 'task'
  try {
    std::string msg_str =
        std::string(message.body(), message.body() + message.bodySize());
    json json_out = json::parse(msg_str);

    if (task == PredictionTask) {
      std::cout << "Prediction Task here\n";
      std::variant<cv::Mat, int> img = get_image(json_out);
      if (std::holds_alternative<cv::Mat>(img)) {
        std::cout << "Image extracted.\n";
      } else if (std::holds_alternative<int>(img)) {
        std::cout << "Error extracting image.\n";
      }
    } else if (task == HelloTask) {
      std::cout << "Hello Task here\n";
      std::variant<std::string, int> hello_msg{get_hello_message(json_out)};
      if (std::holds_alternative<std::string>(hello_msg)) {
        std::cout << "Hello message: " << std::get<std::string>(hello_msg)
                  << std::endl;
      } else if (std::holds_alternative<int>(hello_msg)) {
        std::cout << "Error extracting Hello message.\n";
      } else {
        std::cout << "Unknown task here.";
      }
    }
  } catch (json::parse_error &e) {
    std::cout << "JSON parse error thrown: " << e.what() << "\n";
    std::cout << "JSON parse error thrown \n";
  }
}

// if defined as lambda function, one might access the MyConnectionHandler
// object .onReceived([this](const AMQP::Message &message, uint64_t
// deliveryTag,
//                    bool redelivered) {
//   std::cout << "Message received.\n";
//   std::cout << "Message body" << message.body() << ".\n";
// });

void onErrorCb(const char *message) { std::cout << message; }
