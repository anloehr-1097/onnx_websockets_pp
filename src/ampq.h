#include "ampq_socket.h"
#include "amqpcpp/message.h"
#include <amqpcpp.h>
#include <arpa/inet.h>
#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <sys/socket.h>
#include <utility>

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
    std::cout << "onReady called" << std::endl;
    // create channel, set exchange and queue
    channel = std::make_shared<AMQP::Channel>(connection);
    channel->declareExchange("my-exchange", AMQP::fanout);
    channel->bindQueue("my-exchange", "celery", "celery");
    std::cout << "Channel created." << std::endl;
    std::cout << "Channel ready / usable: " << channel->ready() << " / "
              << channel->usable() << std::endl;
    connection_ready = true;
    channel->publish("my-exchange", "celery", "Hello AMQP, I'm here!\n");
    channel->consume("celery")
        .onSuccess([](const std::string &tag) {
          std::cout << tag << std::endl;
          std::cout << "onSuccess called\n";
        })
        .onData([](const char *data, int64_t len) {
          std::cout << "onData called\n";
          std::cout << "Data: " << data << std::endl;
          // auto jd = json::parse(data);
        })
        .onComplete([](int64_t lg, bool d) {
          std::cout << "onComplete called";
          std::cout << "Long: " << lg << " & bool: " << d << std::endl;
        })
        .onReceived([this](const AMQP::Message &message, uint64_t deliveryTag,
                           bool redelivered) {
          std::cout << "Message received.\n";
          std::cout << "Message body" << message.body() << ".\n";
        });
    std::cout << "Started consuming\n";
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
