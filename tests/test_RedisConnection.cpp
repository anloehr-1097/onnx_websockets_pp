#include "../config.h"
#include <gtest/gtest.h>
#include <hiredis/hiredis.h>
#include <nlohmann/json.hpp>
#include <string>

const std::string redis_connect_error =
    std::string("Cannot connect to redis backend at ") +
    std::string(backend_addr) + ":" + std::to_string(backend_listen_port);

int handle_redis_connect(redisContext *redis) {

  if (redis == NULL || redis->err) {
    if (redis != NULL) {
      printf("Error: %s\n", redis->errstr);
      // handle error
    } else {
      printf("can't allocate redis context\n");
    }

    return 1;
  }
  return 0;
}

TEST(redis_test, test_insert) {
  redisContext *redis = redisConnect(backend_addr.data(), backend_listen_port);
  ASSERT_EQ(handle_redis_connect(redis), 0) << redis_connect_error;
  std::string insert_cmd{"SET celery-task-meta-123 some_return_val"};
  redisReply *reply = (redisReply *)redisCommand(redis, insert_cmd.data());
  std::string rep_str(reply->str, reply->len);
  ASSERT_EQ(rep_str, "OK");
}

// ASSUMES redis is running
TEST(redis_test, test_insert_and_get) {
  // insertion and retrieval of data from redis backend
  redisContext *redis = redisConnect(backend_addr.data(), backend_listen_port);
  ASSERT_EQ(handle_redis_connect(redis), 0) << redis_connect_error;
  std::string insert_cmd{"SET 123 some_return_val"};
  redisReply *ins_reply = (redisReply *)redisCommand(redis, insert_cmd.data());
  std::string rep_str(ins_reply->str, ins_reply->len);
  ASSERT_EQ(rep_str, "OK");

  std::string get_cmd{"GET 123"};
  redisReply *get_reply = (redisReply *)redisCommand(redis, get_cmd.data());
  std::string get_str(get_reply->str, get_reply->len);
  ASSERT_EQ(get_str, "some_return_val");
}

TEST(redis_test, test_insert_and_get_json) {
  redisContext *redis = redisConnect(backend_addr.data(), backend_listen_port);
  ASSERT_EQ(handle_redis_connect(redis), 0) << redis_connect_error;
  std::string val{"1 2 3 4 0.5"};
  nlohmann::json js{val};
  std::string jstr = js.dump();
  std::string key = "123";
  const char *argv[] = {"SET", key.c_str(), jstr.c_str(), NULL};
  size_t argv_len[] = {3, key.size(), jstr.size()};
  redisReply *ins_reply =
      (redisReply *)redisCommandArgv(redis, 3, argv, argv_len);
  std::string rep_str(ins_reply->str, ins_reply->len);
  ASSERT_EQ(rep_str, "OK");

  std::string get_cmd{"GET 123"};
  redisReply *get_reply = (redisReply *)redisCommand(redis, get_cmd.data());
  std::string get_str(get_reply->str, get_reply->len);
  ASSERT_EQ(get_str, jstr);
}
