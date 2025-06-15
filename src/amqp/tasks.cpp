#include "tasks.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;
TaskName determine_tasks(json j) { return HelloTask; }

bool assertTaskCalled(std::string_view &rk, Task &task) {
  return task.routing_key == rk;
}
