#include "callbacks.h"
#include "json_utils.h"
#include <cstdint>
#include <fstream>
#include <ios>
#include <iostream>
#include <memory>
#include <nlohmann/json.hpp>
#include <opencv2/core.hpp>
#include <string>
#include <string_view>

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
  ch->ack(deliveryTag);
  std::cout << "Acknowledged message with tag " << deliveryTag << std::endl;
  std::cout << "Routing key: " << message.routingkey() << std::endl;

  std::string_view s(message.body(), 100);
  std::cout << "Message body first 100 bytes: " << s << ".\n";
  std::cout << "Message content type: " << message.contentType() << ".\n";

  try {
    std::string msg_str =
        std::string(message.body(), message.body() + message.bodySize());

    std::string fname = std::string("json_out") + std::to_string(deliveryTag);
    json json_out = json::parse(msg_str);
    save_json(json_out, fname);
    std::cout << "Json parsing complete, JSON saved." << std::endl;
    if (determine_msg(json_out) == STRING_MSG) {
      std::cout << "JSON string message: " << parse_string_msg(json_out);
    } else if (determine_msg(json_out) == ARRAY_MSG) {
      std::vector<std::string> v;
      parse_array_msg(json_out, v);
      std::cout << "Json array message: ";
      for (auto i : v) {
        std::cout << i << "\t";
      }
      std::cout << std::endl;
      // get first array elem
    } else {

      std::cout << "Json other type of message\n";
    }
    // if data is base64 image

  } catch (json::parse_error &e) {

    std::cout << "JSON parse error thrown: " << e.what() << "\n";
    std::cout << "JSON parse error thrown \n";
  }
}

// if defined as lambda function, one might access the MyConnectionHandler
// object .onReceived([this](const AMQP::Message &message, uint64_t deliveryTag,
//                    bool redelivered) {
//   std::cout << "Message received.\n";
//   std::cout << "Message body" << message.body() << ".\n";
// });
