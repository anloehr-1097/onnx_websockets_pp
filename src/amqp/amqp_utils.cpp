#include "amqp_utils.h"
#include <iostream>
#include <string>

int insert_to_redis(std::string &task_id, json js, redisContext *redis) {

  std::cout << js.dump() << " <-- res backend insert\n";
  std::string ky = "celery-task-meta-" + task_id;
  std::string jstr = js.dump();
  const char *argv[] = {"SET", ky.c_str(), jstr.c_str()};
  size_t argvlen[] = {3, ky.size(), jstr.size()};

  redisReply *reply = (redisReply *)redisCommandArgv(redis, 3, argv, argvlen);
  freeReplyObject(reply);
  return 0;
}
