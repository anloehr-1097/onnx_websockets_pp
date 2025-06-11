#ifndef SRC_AMPQ_H
#define SRC_AMPQ_H

#include "ampq_socket.h"
#include "amqpcpp/flags.h"
#include "callbacks.h"
#include <amqpcpp.h>
#include <arpa/inet.h>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <memory>
#include <nlohmann/json.hpp>
#include <string>
#include <string_view>
#include <sys/socket.h>
#include <utility>

using json = nlohmann::json;
// using json = nlohmann::json;
// You'll need to extend the ConnectionHandler class and make your own, like
// this
class MyConnectionHandler : public AMQP::ConnectionHandler {
  /**
   *  Method that is called by the AMQP library every time it has data
   *  available that should be sent to RabbitMQ.
   *  @param  connection  pointer to the main connection object
   *  @param  data        memory buffer with the data that should be sent to
   * RabbitMQ
   *  @param  size        size of the buffer
   */
public:
  MySocket sock; // socket descriptor
  bool connection_ready = false;
  MyConnectionHandler(MySocket sock) : sock(sock) {}
  std::shared_ptr<AMQP::Channel> channel;
  std::string buf = {};

  void onData(AMQP::Connection *connection, const char *data,
              size_t size) override {
    // @todo
    //  Add your own implementation, for example by doing a call to the
    //  send() system call. But be aware that the send() call may not
    //  send all data at once, so you also need to take care of buffering
    //  the bytes that could not immediately be sent, and try to send
    //  them again when the socket becomes writable again

    // should check if socket is writable, then write
    size_t sent = 0;
    while (sent < size) {
      int res = sock._send(data, size);
      if (res <= 0) {
        std::cerr << "Send error\n";
        break;
      }
      sent += res;
    }
  }

  /**
   *  Method that is called by the AMQP library when the login attempt
   *  succeeded. After this method has been called, the connection is ready
   *  to use.
   *  @param  connection      The connection that can now be used
   */
  void onReady(AMQP::Connection *connection) override {
    // @todo
    //  add your own implementation, for example by creating a channel
    //  instance, and start publishing or consuming
    // std::cout << "onReady called" << std::endl;
    // std::cout << "Max frame size: " << connection->maxFrame() << std::endl;
    // create channel, set exchange and queue
    channel = std::make_shared<AMQP::Channel>(connection);
    channel->declareExchange("celery", AMQP::direct, AMQP::durable);
    channel->declareQueue("celery");
    channel->bindQueue("celery", "celery", "celery");

    // AMQP::Table ch_cfg =
    //     AMQP::Table().set(std::string{"durable"}, AMQP::durable);
    channel->declareExchange("yolo_pred", AMQP::direct, AMQP::durable);
    channel->declareQueue("yolo prediction");
    channel->bindQueue("yolo_pred", "yolo prediction", "yolo_inf");

    std::cout << "Channel created." << std::endl;
    std::cout << "Channel ready / usable: " << channel->ready() << " / "
              << channel->usable() << std::endl;

    connection_ready = true;
    json j_string = "Hello AMQP, I'm here!\n";
    // std::cout << j_string.dump();
    // char *msg_ch = "Hello AMQP, I'm here!\n";
    // auto enc_msg = nlohmann::js
    // AMQP::Envelope msg(std::string_view(msg_ch, strlen(msg_ch)));
    // AMQP::Envelope msg(j_string.dump());
    // AMQP::Table msg_headers = AMQP::Table().set(
    //     std::string("content_type"), std::string_view("application/json"));
    // msg.setContentType(std::string("application/json"));
    std::cout << "Publish Message: " << j_string << std::endl;
    // std::cout << "Publish Message has content type: " << msg.hasContentType()
    //           << std::endl;
    // channel->publish("yolo_pred", "yolo_inf", j_string.dump());
    channel->publish("celery", "celery", j_string.dump());

    channel->consume("celery")
        .onSuccess(onSuccessCb)
        .onData([this](const char *data, int64_t len) {
          onDataCb(this->buf, data, len);
        })
        .onComplete(onCompleteCb)
        .onReceived([this](const AMQP::Message &message, uint64_t deliveryTag,
                           bool redelivered) {
          onReceivedCb(this->channel, message, deliveryTag, redelivered);
        });

    std::cout << "Started consuming celery queue\n";
    channel->consume("yolo prediction")
        .onSuccess(onSuccessCb)
        .onData([this](const char *data, int64_t len) {
          onDataCb(this->buf, data, len);
        })
        .onComplete(onCompleteCb)
        .onReceived([this](const AMQP::Message &message, uint64_t deliveryTag,
                           bool redelivered) {
          onReceivedCb(this->channel, message, deliveryTag, redelivered);
        });

    std::cout << "Started consuming yolo inf queue\n";
  }

  /**
   *  Method that is called by the AMQP library when a fatal error occurs
   *  on the connection, for example because data received from RabbitMQ
   *  could not be recognized.
   *  @param  connection      The connection on which the error occurred
   *  @param  message         A human readable error message
   */
  void onError(AMQP::Connection *connection, const char *message) override {
    std::cout << "onError called" << std::endl;
    std::cout << "Message: " << message << std::endl;
    // @todo
    //  add your own implementation, for example by reporting the error
    //  to the user of your program, log the error, and destruct the
    //  connection object because it is no longer in a usable state
  }

  /**
   *  Method that is called when the connection was closed. This is the
   *  counter part of a call to Connection::close() and it confirms that the
   *  AMQP connection was correctly closed.
   *
   *  @param  connection      The connection that was closed and that is now
   * unusable
   */
  void onClosed(AMQP::Connection *connection) override {
    std::cout << "onClosed called" << std::endl;
    // @todo
    //  add your own implementation, for example by closing down the
    //  underlying TCP connection too
  }
  uint16_t onNegotiate(AMQP::Connection *connection,
                       uint16_t interval) override {
    uint16_t _five = 5;
    return _five;
  }
};

#endif // SRC_AMPQ_H
