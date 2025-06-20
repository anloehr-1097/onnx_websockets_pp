// These callbacks are used in the connection handler class.
#ifndef SRC_CALLBACKS_H_
#define SRC_CALLBACKS_H_

#include "../OnnxInferLib/include/inference.h"
#include "../OnnxInferLib/include/onnx_config.h"
#include "../OnnxInferLib/include/types.h"
#include "hiredis/hiredis.h"
#include <amqpcpp.h>
#include <cstdint>
#include <memory>
#include <string>

// callback when chunk of data gets delivered using connection
void onDataCb(std::string &buf, const char *message, int64_t len);

// callback when consumer started consuming message
void onSuccessCb(const std::string &consumer_tag);

// funtion to be called when a message was completely received
void onCompleteCb(std::string &buffer, int64_t deliveryTag, bool redelivered);

// function called once complete message has been received
void onReceivedCb(std::shared_ptr<AMQP::Channel> ch,
                  const AMQP::Message &message, uint64_t deliveryTag,
                  bool redelivered);
void onReceivedPredCb(std::shared_ptr<AMQP::Channel> ch,
                      std::shared_ptr<Yolov11Session> sess,
                      std::shared_ptr<redisContext> redis,
                      const AMQP::Message &message, uint64_t deliveryTag,
                      bool redelivered);

// function called when channel
void onErrorCb(const char *message);

#endif // SRC_CALLBACKS_H_
