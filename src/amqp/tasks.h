#ifndef SRC_TASKS_H
#define SRC_TASKS_H
#include <nlohmann/json.hpp>
enum Task { HelloTask, PredictionTask, UnknownTask };

Task determine_task(nlohmann::json);

#endif // SRC_TASKS_H
