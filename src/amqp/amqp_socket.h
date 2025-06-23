#ifndef SRC_AMPQ_SOCKET
#define SRC_AMPQ_SOCKET
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>

#include <algorithm>
#include <iostream>
#include <memory>
#include <string>
#include <string_view>

class MySocket {
  struct sockaddr_in server;   // info on server to connect to
  std::shared_ptr<int> _sock;  // socket in use by class

 public:
  char buffer[1000000] = {0};
  explicit MySocket(int port = 5672, const char *address = "127.0.0.1");
  int connect();
  int close();

  /*
   * Check if *_sock is readable.
   * Return Values:
   * -1 in case underlying 'select' syscall failed
   *  0 if timeout is reached or no file descriptor is readable
   *  1 if socket is readable
   * */
  int readable(timeval &timeout);

  void reset_buf();
  void print_buf(size_t num_bytes);
  int send(const char *message, size_t len);
  int receive();
};

#endif  // SRC_AMPQ_SOCKET
