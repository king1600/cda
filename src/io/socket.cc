#include "socket.hh"
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

/* DNS Resolve a hostname with the port
 * @param {const char*} host the hostname
 * @param {int} port the port to use as service
 * @param {char*} out the resolved ipv4 address output
 */
int io::Resolve(const char *host, int port, char *addr)
{
  // setup variables
  int ret;
  struct addrinfo *i;
  struct addrinfo hints;
  struct addrinfo *result;

  // create address hints
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = 0;

  // perform resulutions
  ret = getaddrinfo(host,std::to_string(port).c_str(),
    &hints, &result);
  if (ret != 0) return -1;

  // get first result
  for (i = result; i != nullptr; i = i->ai_next) {
    if (inet_ntop(AF_INET,
      &((struct sockaddr_in*)i->ai_addr)->sin_addr
      , addr, 16) == nullptr) continue;
    break;
  }

  // free address info and return success state
  freeaddrinfo(result);
  return (addr == nullptr) ? -1 : 0;
}

/**
 * Close the socket connection
 * @param {int} err the error code when closing
 */
void io::Socket::Close(int err) {
  if (closed) return;
  close(fd);
  if (ssl != nullptr) {
    SSL_shutdown(ssl);
    SSL_free(ssl);
  }
  closed = false;
  close_cb(err);
}