#include "callbacks.h"
#include "inference.h"
#include "json_utils.h"
#include "tasks.h"
#include "types.h"
#include "utils.h"
#include <cstdint>
#include <cstring>
#include <fstream>
#include <hiredis/hiredis.h>
#include <ios>
#include <iostream>
#include <memory>
#include <nlohmann/json.hpp>
#include <opencv2/core.hpp>
#include <string>
#include <string_view>
#include <variant>

using json = nlohmann::json;

void onDataCb(std::string &buf, const char *data, int64_t len) {
  std::cout << "onData called\nBuffer: ";
  buf.append(data);
  std::cout << buf << std::endl;
}

void onSuccessCb(const std::string &consumer_tag) {
  std::cout << "onSuccess called with tag " << consumer_tag << "\n";
}

void onCompleteCb(std::string &buf, int64_t delivery_tag, bool redelivered) {
  std::cout << "onComplete called\n";
  std::cout << "deliveryTag: " << delivery_tag
            << " redelivered: " << redelivered << std::endl;
  buf.clear();
  std::cout << "Buffer after complete transmission: " << buf << std::endl;
}
void onReceivedCb(std::shared_ptr<AMQP::Channel> ch,
                  const AMQP::Message &message, uint64_t deliveryTag,
                  bool redelivered) {
  // Ack message such that no other worker consumes it while processing
  std::cout << "onReceivedCb called" << std::endl;
  ch->ack(deliveryTag);
  std::cout << "Acknowledged message with tag " << deliveryTag << std::endl;
  std::string_view rk{message.routingkey()};
  std::cout << "Routing key: " << rk << std::endl;
  Greet greetTask{};
  if (assertTaskCalled(rk, greetTask) != 0) {
    std::cerr << "Wrong task in callback.\n";
  }
  // parse json based on value of 'task'
  try {
    std::string msg_str =
        std::string(message.body(), message.body() + message.bodySize());
    json json_out = json::parse(msg_str);
    std::variant<std::string, int> hello_msg{get_hello_message(json_out)};

    if (std::holds_alternative<std::string>(hello_msg)) {
      std::cout << "Hello message: " << std::get<std::string>(hello_msg)
                << std::endl;
    } else if (std::holds_alternative<int>(hello_msg)) {
      std::cout << "Error extracting Hello message.\n";
    }
    // if (task == PredictionTask) {
    //   std::cout << "Prediction Task here\n";
    //   std::variant<cv::Mat, int> img = get_image(json_out);
    //   if (std::holds_alternative<cv::Mat>(img)) {
    //     std::cout << "Image extracted.\n";
    //   } else if (std::holds_alternative<int>(img)) {
    //     std::cout << "Error extracting image.\n";
    //   }
    //
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

void onReceivedPredCb(std::shared_ptr<AMQP::Channel> ch,
                      std::shared_ptr<Yolov11Session> sess,
                      std::shared_ptr<redisContext> redis_storage,
                      const AMQP::Message &message, uint64_t deliveryTag,
                      bool redelivered) {

  ch->ack(deliveryTag);
  std::cout << "Acknowledged message with tag " << deliveryTag << std::endl;
  std::string_view rk{message.routingkey()};
  std::string task_id = message.headers().get("id");
  std::cout << "Task id: " << task_id << std::endl;
  std::cout << "Routing key: " << rk << std::endl;
  Predict predTask{};
  if (assertTaskCalled(rk, predTask) != 0) {
    std::cout << "Wrong task, aborting\n";
    return;
  }
  // parse json based on value of 'task'
  try {
    std::string msg_str =
        std::string(message.body(), message.body() + message.bodySize());
    json json_out = json::parse(msg_str);

    std::variant<cv::Mat, int> img = get_image(json_out);
    if (std::holds_alternative<cv::Mat>(img)) {
      std::cout << "Image extracted.\n";
      img = preprocess_img(std::get<cv::Mat>(img), cv::Size(640, 640), 0);
      sess->set_input_image(std::get<cv::Mat>(img));
      auto out_tens = sess->detect();
      auto res = sess->postprocess(out_tens);
      // std::string sess_string{"45"};
      std::string sess_string;
      for (ObbDetection f : res) {
        sess_string.append(f.to_string());
      }
      nlohmann::json js_resp =
          write_celery_result_to_redis(task_id, sess_string);
      std::cout << js_resp.dump() << " <-- res backend insert\n";
      std::string ky = "celery-task-meta-" + task_id;
      std::string cmd =
          "SET celery-task-meta-" + task_id + " " + js_resp.dump();
      // const char *chcmd = cmd.c_str();
      std::string jstr = js_resp.dump();
      js_resp.dump();
      const char *argv[] = {"SET", ky.c_str(), jstr.c_str()};
      size_t argvlen[] = {3, ky.size(), jstr.size()};
      redisReply *reply =
          (redisReply *)redisCommandArgv(redis_storage.get(), 3, argv, argvlen);
      freeReplyObject(reply);

    } else if (std::holds_alternative<int>(img)) {
      std::cout << "Error extracting image.\n";
    }
  } catch (json::parse_error &e) {
    std::cout << "JSON parse error thrown: " << e.what() << "\n";
    std::cout << "JSON parse error thrown \n";
  }
}
