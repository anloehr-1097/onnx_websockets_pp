#include "config.h"
#include <amqp_socket.h>
#include <amqpcpp.h>
#include <conn_handler.h>
#include <filesystem>
#include <iostream>
#include <string>
#include <sys/time.h>

std::string parse_cmd_args(int argc, char **argv) {
  if (argc == 1) {
    // first arg is program name
    return "";
  } else if (argc == 2) {
    // second arg: mode
    return argv[1];
  } else {
    return "";
  }
}

void run_event_loop(MySocket &sock, AMQP::Connection &connection) {
  /* Run event loop, monitoring socket.
   * At frequent intervals, send heartbeats
   * */
  int poll_status = 0;
  ssize_t received = 0;
  size_t parsed_bytes = 0;
  timeval timeout;
  timeval start, end;
  timeout.tv_usec = 0;
  timeout.tv_sec = 0;
  int64 elapsed = 0;

  // basic event loop
  while (true) {
    // every iteration check if socket is readable, if so read into buffer
    // Every couple of seconds, send a heartbeat to the Rabbit-MQ broker
    if (gettimeofday(&start, nullptr) != 0) {
      std::cerr << "Cannot get current time. Exiting..." << std::endl;
      exit(-1);
    }
    // at each iteration, clear data buffer of MySocket, parsed bytes
    received = 0;
    parsed_bytes = 0;
    sock.reset_buf();

    // check if socket is readable
    poll_status = sock.readable(timeout);

    if (poll_status == 1) {
      // socket is readable, ready to receive message
      received = sock._receive();
      if (received > 0) {
        // Pass incoming data to the AMQP connection
        while (parsed_bytes < received) {
          // parse as long as there is unparsed data
          parsed_bytes = connection.parse(sock.buffer + parsed_bytes,
                                          received - parsed_bytes);
        }

      } else if (received == 0) {
        std::cout << "Server closed connection\n";
        break;
      } else {
        std::cerr << "Recv error\n";
        break;
      }
      // read
    } else if (poll_status < 1) {
    }
    gettimeofday(&end, nullptr);
    // check if hearbeat must be sent
    elapsed +=
        (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_usec - start.tv_usec);
    if (elapsed > 3 * 1000000) {
      connection.heartbeat();
      elapsed = 0;
    }
  }
}

int main(int argc, char **argv) {
  auto mpath = std::filesystem::path{model_path.data()};
  auto mode = parse_cmd_args(argc, argv);

  std::cout << "Mode: " << mode << std::endl;
  if (mode == "AMQP") {
    // create socket underlying AMQP connection used by connection handler
    MySocket sock(broker_listen_port, broker_addr.data());
    sock.do_connect(); // connect to broker Rabbit MQ
    std::string_view ba(backend_addr);
    MyConnectionHandler myHandler(sock, mpath, ba, backend_listen_port);
    // create a AMQP connection object
    AMQP::Connection connection(&myHandler, AMQP::Login("guest", "guest"), "/");
    run_event_loop(sock, connection);

  } else if (mode == "websockets") {
  } else {
    std::cerr << "No mode provided\n";
  }
  return 0;
}
