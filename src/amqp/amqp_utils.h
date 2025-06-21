#ifndef SRC_AMQP_UTILS_H
#define SRC_AMQP_UTILS_H
#include <hiredis/hiredis.h>
#include <nlohmann/json.hpp>
using json = nlohmann::json;

int insert_to_redis(std::string &task_id, json js, redisContext *redis);

#endif // SRC_AMQP_UTILS_H
