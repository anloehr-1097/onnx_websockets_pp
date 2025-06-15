#ifndef SRC_TASKS_H
#define SRC_TASKS_H
#include "base64.h"
#include "json_utils.h"
#include <iostream>
#include <nlohmann/json.hpp>
#include <string>
#include <string_view>
enum TaskName { HelloTask, PredictionTask, UnknownTask };
using json = nlohmann::json;
TaskName determine_task(nlohmann::json);

struct Task {
  TaskName name;
  std::string_view queue_name;
  std::string_view exchange_name;
  std::string_view routing_key;

  virtual std::variant<std::string, int> parse(nlohmann::json &) = 0;
  virtual void callback() = 0;
  virtual void postprocess() = 0;
};

struct Predict : Task {
  TaskName name = PredictionTask;
  const std::string_view queue_name{"yolo prediction"};
  const std::string_view exchange_name{"yolo_pred"};
  const std::string_view routing_key{queue_name};

  Predict() {}

  std::variant<std::string, int> parse(json &js) override {
    if (!js[0][0].contains("__type__")) {
      return -1;
    }
    std::string tp{js[0][0]["__type__"]};
    if (tp != "base64") {
      return -1;
    }

    if (!js[0][0].contains("__value__")) {
      return -1;
    }
    std::string b64_str{js[0][0]["__value__"]};
    return base64_decode(b64_str);
  }
  void callback() override { std::cout << "Callback\n"; }
  void postprocess() override { std::cout << "Postprocess\n"; }
};

struct Greet : Task {
  TaskName name = HelloTask;

  const std::string_view queue_name{"celery"};
  const std::string_view exchange_name{"celery"};
  const std::string_view routing_key{"celery"};
  Greet() { std::cout << "Hello Greet\n"; }

  std::variant<std::string, int> parse(nlohmann::json &js) {
    return get_hello_message(js);
  }
  void callback() override { std::cout << "Callback in Greet"; }
  void postprocess() override { std::cout << "Postprocess in Greet"; }
};
#endif // SRC_TASKS_H
//
//
//
bool assertTaskCalled(std::string_view &rk, Task &task);
