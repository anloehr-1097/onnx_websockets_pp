#include "callbacks.h"
#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <string_view>
void onDataCb(std::string &buf, const char *data, int64_t len) {

  std::cout << "onData called\n";
  std::cout << "First 100 bytes of data: " << std::string(data, 100) + "..."
            << std::endl;
  buf.append(data);
  // std::cout << "Buffer: " << buf << std::endl;
}

void onSuccessCb(const std::string &tag) {
  std::cout << "onSuccess called with tag " << tag << "\n";
}

void onCompleteCb(int64_t lg, bool d) {
  std::cout << "onComplete called\n";
  std::cout << "Long: " << lg << " & bool: " << d << std::endl;
}
void onReceivedCb(std::shared_ptr<AMQP::Channel> ch,
                  const AMQP::Message &message, uint64_t deliveryTag,
                  bool redelivered) {
  ch->ack(deliveryTag);
  std::cout << "Acknowledged message with tag " << deliveryTag << std::endl;
  std::string_view s(message.body(), 100);
  std::cout << "Message body first 100 bytes: " << s << ".\n";
}

// if defined as lambda function, one might access the MyConnectionHandler
// object .onReceived([this](const AMQP::Message &message, uint64_t deliveryTag,
//                    bool redelivered) {
//   std::cout << "Message received.\n";
//   std::cout << "Message body" << message.body() << ".\n";
// });
