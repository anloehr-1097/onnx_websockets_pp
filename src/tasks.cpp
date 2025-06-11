#include "tasks.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;
Task determine_tasks(json j) { return HelloTask; }
