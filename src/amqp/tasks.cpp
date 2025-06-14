#include "tasks.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;
TaskName determine_tasks(json j) { return HelloTask; }
