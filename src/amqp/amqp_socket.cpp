#include "amqp_socket.h"

#include <cerrno>
#include <iostream>

MySocket::MySocket(int port, const char *address) {
  // create the underlying socket connection
  _sock = std::make_shared<int>(socket(AF_INET, SOCK_STREAM, 0));
  if (_sock < 0) {
    std::cerr << "Could not create socket." << std::endl;
    exit(1);
  }
  server.sin_family = AF_INET;
  server.sin_port = htons(port);
  inet_pton(AF_INET, address, &server.sin_addr);
}

int MySocket::connect() {
  if (::connect(*_sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
    std::cerr << "Failed to connect" << std::endl;
    return 0;
  }
  return 1;
}

int MySocket::close() { return ::close(*_sock); }

int MySocket::send(const char *message, size_t size) {
  int res = ::send(*_sock, message, size, 0);
  if (res == -1) {
    std::cerr << "Send error with code " << errno << std::endl;
  }
  return res;
}

int MySocket::receive() {
  // TODO(andy) handler larger messages? How to choose buffer size?
  int received = recv(*_sock, this->buffer, sizeof(this->buffer), 0);
  if (received < 0) {
    std::cerr << "recv call failed with error " << errno << std::endl;
  }
  return received;
}

int MySocket::readable(timeval &timeout) {
  fd_set readfds;
  int ret;
  // Clear the set and add *_sock file descriptor
  FD_ZERO(&readfds);
  FD_SET(*_sock, &readfds);

  ret = select(*_sock + 1, &readfds, NULL, NULL, &timeout);

  if (ret == -1) {
    std::cerr << "Select failed with errorno " << errno << std::endl;
    return -1;
  } else if (ret == 0) {
    return 0;
  } else {
    if (FD_ISSET(*_sock, &readfds)) {
      return 1;
    } else {
      return 0;
    }
  }
}

void MySocket::reset_buf() { std::fill(buffer, buffer + sizeof(buffer), 0); }
void MySocket::print_buf(size_t num_bytes) {
  std::cout << "Printinng " << num_bytes << " bytes from internal buffer."
            << std::endl;
  for (ssize_t i = 0; i < num_bytes; ++i) {
    std::cout << static_cast<unsigned char>(buffer[i]);
  }
  std::cout << std::endl;
}
