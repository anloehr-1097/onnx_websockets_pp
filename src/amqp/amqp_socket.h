#ifndef SRC_AMPQ_SOCKET
#define SRC_AMPQ_SOCKET
#include <algorithm>
#include <arpa/inet.h>
#include <iostream>
#include <memory>
#include <netinet/in.h>
#include <string>
#include <string_view>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 5672
// #define PORT 15692

class MySocket {
  struct sockaddr_in server;  // info on server to connect to
  std::shared_ptr<int> _sock; // socket in use by class

public:
  char buffer[1000000] = {0};
  explicit MySocket(int port = 5672, const char *address = "127.0.0.1") {
    _sock = std::make_shared<int>(socket(AF_INET, SOCK_STREAM, 0));
    if (_sock < 0) {
      std::cerr << "Could not create socket" << std::endl;
    }
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    inet_pton(AF_INET, address, &server.sin_addr);
  }

  int do_connect() {
    if (connect(*_sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
      std::cerr << "Failed to connect" << std::endl;
      return 0;
    }

    std::cout << "Connect succesful" << std::endl;
    return 1;
  }

  int close_connection() { return -1; }

  int _send(const char *message, size_t size) {
    int res = send(*_sock, message, size, 0);
    return res;
  }

  ssize_t _receive() {
    ssize_t received = recv(*_sock, this->buffer, sizeof(this->buffer), 0);
    return received;
  }

  int readable(timeval &timeout) {
    fd_set readfds;
    // fd_set writefds;
    // struct timeval timeout;
    int ret;

    // Set timeout to 0 seconds
    // Clear the set and add socket file descriptor
    FD_ZERO(&readfds);
    // FD_ZERO(&writefds);
    FD_SET(*_sock, &readfds);
    // FD_SET(*_sock, &writefds);

    ret = select(*_sock + 1, &readfds, NULL, NULL, &timeout);

    if (ret == -1) {
      std::cerr << "Select failed\n";
      return -1;
    } else if (ret == 0) {
      return 0;
    } else {
      if (FD_ISSET(*_sock, &readfds)) {
        return 1;
      } else {
        return -2;
      }
    }
  }

  void reset_buf() { std::fill(buffer, buffer + sizeof(buffer), 0); }
  void print_buf(size_t num_bytes) {

    std::cout << "Received " << num_bytes << " bytes." << std::endl;
    for (ssize_t i = 0; i < num_bytes; ++i) {
      printf("%02x ", static_cast<unsigned char>(buffer[i]));
    }
    printf("\n");
  }
  void close() { ::close(*_sock); }
};

#endif // SRC_AMPQ_SOCKET
