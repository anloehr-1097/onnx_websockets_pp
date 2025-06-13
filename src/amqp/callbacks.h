#ifndef SRC_CALLBACKS_H_
#define SRC_CALLBACKS_H_

#include "../OnnxInferLib/include/inference.h"
// #include "amqpcpp.h"
#include <amqpcpp.h>
#include <cstdint>
#include <memory>
#include <string>

void onDataCb(std::string &, const char *, int64_t);
void onSuccessCb(const std::string &);
void onCompleteCb(int64_t, bool);
void onReceivedCb(std::shared_ptr<AMQP::Channel>, const AMQP::Message &,
                  uint64_t, bool);
void onReceivedPredCb(std::shared_ptr<AMQP::Channel> ch,
                      std::shared_ptr<Yolov11Session> sess,
                      const AMQP::Message &message, uint64_t deliveryTag,
                      bool redelivered);
void onErrorCb(const char *message);

#endif // SRC_CALLBACKS_H_
