#ifndef SRC_AMPQ_H
#define SRC_AMPQ_H

#include "../config.h"
#include "../utils/json_utils.h"
#include "amqp_socket.h"
#include "callbacks.h"
#include "hiredis/hiredis.h"
#include "inference.h"
#include <amqpcpp.h>
#include <arpa/inet.h>
#include <cstdint>
#include <cstring>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <string_view>
#include <sys/socket.h>
#include <unistd.h>

class MyConnectionHandler : public AMQP::ConnectionHandler {
public:
  MySocket sock; // socket descriptor
  std::shared_ptr<AMQP::Channel> channel;
  std::string buf = {};
  std::shared_ptr<Yolov11Session> onnx_sess;
  std::shared_ptr<redisContext> redis;

  MyConnectionHandler(MySocket sock, std::filesystem::path fp,
                      std::string_view &backend_address, int backend_port)
      : sock(sock), onnx_sess(std::make_shared<Yolov11Session>(fp)),
        redis(std::shared_ptr<redisContext>(
            redisConnect(backend_address.data(), backend_port))) {}

  /**
   *  Method that is called by the AMQP library every time it has data
   *  available that should be sent to RabbitMQ.
   *  @param  connection  pointer to the main connection object
   *  @param  data        memory buffer with the data that should be sent to
   * RabbitMQ
   *  @param  size        size of the buffer
   */
  void onData(AMQP::Connection *connection, const char *data,
              size_t size) override {
    // @todo
    //  Add your own implementation, for example by doing a call to the
    //  send() system call. But be aware that the send() call may not
    //  send all data at once, so you also need to take care of buffering
    //  the bytes that could not immediately be sent, and try to send
    //  them again when the socket becomes writable again

    // TODO(andy) should check if socket is writable, then write
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
    // std::cout << "Max frame size: " << connection->maxFrame() << std::endl;
    // create channel, set exchange and queue
    channel = std::make_shared<AMQP::Channel>(connection);
    register_channel_callbacks();

    // setup exchanges, along with queues and their respective routing keys
    int status = 0;
    if ((status = setup_exchange_and_queue_routing(channel, "celery", "celery",
                                                   "celery")) != 0) {
      std::cerr << "Failed setting up exchange & key.\n";
    }

    if ((status = setup_exchange_and_queue_routing(
             channel, "yolo_pred", "yolo prediction", "yolo_inf")) != 0) {

      std::cerr << "Failed setting up exchange & key.\n";
    };

    // publish test message
    std::string_view j_string = "Hello AMQP, I'm here!\n";
    channel->publish("celery", "celery", to_js_string(j_string).dump());

    // start consumption on channels defined above
    channel->consume("celery")
        .onSuccess(onSuccessCb)
        .onData([this](const char *data, int64_t len) {
          onDataCb(this->buf, data, len);
        })
        .onComplete([this](int64_t deliveryTag, bool redelivered) {
          onCompleteCb(this->buf, deliveryTag, redelivered);
        })
        .onReceived([this](const AMQP::Message &message, uint64_t deliveryTag,
                           bool redelivered) {
          onReceivedCb(this->channel, message, deliveryTag, redelivered);
        });

    channel->consume("yolo prediction")
        .onSuccess(onSuccessCb)
        .onData([this](const char *data, int64_t len) {
          onDataCb(this->buf, data, len);
        })
        .onComplete([this](int64_t deliveryTag, bool redelivered) {
          onCompleteCb(this->buf, deliveryTag, redelivered);
        })
        .onReceived([this](const AMQP::Message &message, uint64_t deliveryTag,
                           bool redelivered) {
          onReceivedPredCb(this->channel, this->onnx_sess, this->redis, message,
                           deliveryTag, redelivered);
        });
  }

  void register_channel_callbacks() {
    channel->onReady(
        [this] { std::cout << "Channel is ready: " << channel->ready(); });
    channel->onError(
        [](const char *msg) { std::cout << "Channel error: " << msg; });
  }

  int setup_exchange_and_queue_routing(std::shared_ptr<AMQP::Channel>,
                                       std::string_view exchange_name,
                                       std::string_view qname,
                                       std::string_view routing_key) {

    channel->declareExchange(exchange_name, AMQP::direct, AMQP::durable);
    channel->declareQueue(qname);
    channel->bindQueue(exchange_name, qname, routing_key);
    return 0;
  }
  /**
   *  Method that is called by the AMQP library when a fatal error occurs
   *  on the connection, for example because data received from RabbitMQ
   *  could not be recognized.
   *  @param  connection      The connection on which the error occurred
   *  @param  message         A human readable error message
   */
  void onError(AMQP::Connection *connection, const char *message) override {
    std::cout << "onError called with message " << message << std::endl;
    connection->close();
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
    sock.close();
    exit(-1);

    // @todo
    //  add your own implementation, for example by closing down the
    //  underlying TCP connection too
  }
  uint16_t onNegotiate(AMQP::Connection *connection,
                       uint16_t interval) override {
    // negotiate heartbeat interval
    // uint16_t _five = 5;
    return heartbeat_interval; // from src/config.h
  }
};

#endif // SRC_AMPQ_H
