#ifndef SRC_TASKS_H
#define SRC_TASKS_H
#include "base64.h"
#include <nlohmann/json.hpp>
#include <string>
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
  std::string_view queue_name{"yolo prediction"};
  std::string_view exchange_name{"yolo_pred"};
  std::string_view routing_key{"yolo_inf"};

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
};

struct Greet : Task {
  TaskName name = PredictionTask;
  std::string_view queue_name{"celery"};
  std::string_view exchange_name{"celery"};
  std::string_view routing_key{"celery"};
};
#endif // SRC_TASKS_H
