#ifndef SRC_CALLBACKS_H_
#define SRC_CALLBACKS_H_

#include "amqpcpp.h"
#include <cstdint>
#include <memory>
#include <string>

void onDataCb(std::string &, const char *, int64_t);
void onSuccessCb(const std::string &);
void onCompleteCb(int64_t, bool);
void onReceivedCb(std::shared_ptr<AMQP::Channel>, const AMQP::Message &,
                  uint64_t, bool);

#endif // SRC_CALLBACKS_H_
