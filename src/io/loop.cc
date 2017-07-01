#include "loop.hh"
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <cerrno>
#include <stdexcept>
#include <openssl/err.h>

// max amount of socket events
#define MAXEVENTS 128

/**
 * Set socket to non-blocking mode
 * @param {int} fd the socket file descriptor to set
 */
static inline int nonblock(int fd) {
  int flags = fcntl(fd, F_GETFL, 0);
  if (flags == -1) return -1;
  flags |= O_NONBLOCK;
  flags = fcntl(fd, F_SETFL, flags);
  return (flags != 0) ? -1 : 0;
}

/**
 * Initalize an event loop
 */
io::Loop::Loop() {
  // create epoll file descriptor
  epoll = epoll_create1(0);
  if (epoll == -1)
    throw std::runtime_error("Epoll init failed");
  
  // load ssl libraries
  SSL_load_error_strings();
  if (SSL_library_init() < 0)
    throw std::runtime_error("SSL init failed");

  // create shared ssl context for ssl connections
  ctx = SSL_CTX_new(SSLv23_client_method());
  if (ctx == nullptr)
    throw std::runtime_error("SSL Ctx init failed");
}

/**
 * Modify epoll triggers on a socket file descriptor
 * @param {int} fd the socket file descriptor
 * @param {int} op the EPOLL_CTL operation to perform
 * @param {int} flags the EPOLL flags to set
 * @param {void*} data optional data to attach
 */
int io::Loop::mod(int fd, int op, int flags, void *data)
{
  struct epoll_event event;
  event.data.fd = fd;
  event.data.ptr = data;
  event.events = flags;
  if (epoll_ctl(epoll, op, fd, &event) == -1)
    return -1;
  return 0;
}

/**
 * Send data into write queue to be written
 * @param {Data&} data the buffer to enqueue
 */
void io::Socket::Write(io::Data &data) {
  writeQueue.push_front(data);
  int ret = this->loop->mod(fd, EPOLL_CTL_MOD,
    EPOLLIN | EPOLLOUT | EPOLLET,
    this);
  if (ret != 0) Close(ret);
}

/**
 * set socket into connected state
 */
void io::Socket::setConnected() {
  connected = true;
  loop->mod(fd, EPOLL_CTL_MOD,
    EPOLLIN | EPOLLOUT | EPOLLET, this);
  connect_cb();
}

/**
 * Spawn a TCP Socket connection using the uri provided
 * @param {Uri} uri the uri to connect with
 * @return {Socket} the socket object if success else nullptr
 */
io::Socket* io::Loop::spawn(Uri uri) {
  io::Socket* sock = nullptr; // create empty socket object

  // resolve hostname from uri
  char ip[17] = { 0 };
  if (io::Resolve(uri.host.c_str(), uri.port, ip) != 0)
    return sock;
  
  // create socket
  int fd = socket(AF_INET, SOCK_STREAM, 0);
  if (fd < 0) return sock;

  // set socket to non-blocking / async
  if (nonblock(fd) != 0) {
    close(fd);
    return sock;
  }

  // set TCP no-delay for better performance
  int ret = 1;
  if (setsockopt(fd, SOL_TCP, TCP_NODELAY, &ret, sizeof(ret))) {
    close(fd);
    return sock;
  }

  // connect to hostname using uri
  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_port = htons(uri.port);
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = inet_addr(ip);
  ret = connect(fd, (struct sockaddr*)&addr, sizeof(addr));
  if (ret < 0 && errno != EINPROGRESS) {
    close(fd);
    return sock;
  }

  // create socket object
  sock = new io::Socket(fd, this);

  // create ssl info if need be
  if (uri.ssl) {

    // create ssl object
    sock->ssl = SSL_new(ctx);
    if (sock->ssl == nullptr) {
      delete sock;
      return nullptr;
    }

    // set ssl object file descriptor
    if (!SSL_set_fd(sock->ssl, sock->fd)) {
      delete sock;
      return nullptr;
    }

    // set the connection state
    SSL_set_connect_state(sock->ssl);
  }

  // register socket object to listen for events
  ret = mod(fd, EPOLL_CTL_ADD, EPOLLIN | EPOLLOUT | EPOLLET, sock);
  if (ret != 0) {
    delete sock;
    return nullptr;
  }

  // return created socket object
  return sock;
}

/**
 * Check if socket is connected using file descriptor
 * @param {int} fd the socket file descriptor
 * @return {int} 0 if connected else -1;
 */
static inline int isConnected(int fd) {
  int res;
  socklen_t len = sizeof(res);
  if (getsockopt(fd, SOL_SOCKET, SO_ERROR, &res, &len) < 0)
    return -1;
  return res == 0 ? -1 : 0;
}

/**
 * Perform SSL handshake
 * @param {Socket} sock the socket object to handshake with
 * @return {int} 0 if success 1 if continue -1 if error
 */
static inline int sslHandshake(io::Socket *sock) {
  int err = SSL_do_handshake(sock->ssl); // dp handshake

  // get error code to check state
  if (err == 1) return 0;
  err = SSL_get_error(sock->ssl, err);

  // modify epoll state based on error code
  if (err == SSL_ERROR_WANT_WRITE)
    sock->loop->mod(sock->fd, EPOLL_CTL_MOD,
      EPOLLOUT | EPOLLET, sock);
  else if (err == SSL_ERROR_WANT_READ)
    sock->loop->mod(sock->fd, EPOLL_CTL_MOD,
      EPOLLIN | EPOLLET, sock);
  else 
    return -1;

  // try handshake again
  return 1;
}

/**
 * Start the io event loop
 * @return {int} for ease of use
 */
int io::Loop::run() {
  int i, polled;              // poll event iterators
  io::Socket *sock;           // each socket object found
  struct epoll_event event;   // each event found
  struct epoll_event *events; // all events gathered

  /************ IO Objects ***************/
  ssize_t nread, nwrite;      // read/write amounts
  char rbuf[64 * 1024] = {0}; // read buffer
  
  /*********** TIMER VARIABLES ***********/
  long since;               // time passed since timer task
  io::Promise promise;      // iteration promise task
  int size = tasks.size();  // size of current amount of timer tasks
  io::Duration passed;      // time passed in chrono

  // create events holder
  events = (epoll_event*)calloc(MAXEVENTS, sizeof(events));
  if (events == nullptr) return -1;

  // start event loop
  running = true;
  while (running) {

    // wait for socket events
    polled = epoll_wait(epoll, events, MAXEVENTS, 10);

    // Iterate only through current timer tasks
    for (i = 0; i < size && !tasks.empty(); i++) {
      promise = std::move(tasks.front());
      tasks.pop();

      // Do not perform cancelled timer tasks
      if (promise.cancelled) continue;

      // If time passed since timeout, perform task. Else, add back to tasks
      passed = io::Clock::now() - promise.created;
      since = passed.count() * 1000L;
      if (since >= promise.delay)
        promise.callback();
      else
        tasks.push(promise);
    }

    // iterate through found socket events
    for (i = 0; i < polled; i++) {
      event = events[i];
      sock = (io::Socket*)event.data.ptr;

      // kill socket if epoll error
      if (event.events & EPOLLERR || event.events & EPOLLHUP) {
        delete sock;
        continue;
      }

      // socket is ready to write
      if (event.events & EPOLLOUT) { 

        // check if write event was actually a connect event
        if (!sock->connected) {

          // check if socket is connected
          if (!isConnected(sock->fd)) {
            sock->Close(1);
            continue;
          }

          // if normal socket, set connected and continue
          if (sock->ssl == nullptr) {
            sock->setConnected();
            continue;

          // if ssl socket, start ssl handshake
          } else {
            if (sslHandshake(sock) == 0)
              sock->setConnected();
            else continue;
          }
        }

        // since write event, check if theres anythign to be written
        if (sock->hasBuffer()) {
          io::Data buffer;

          // iterate through all items in write queue
          while (sock->hasBuffer()) {
            buffer = sock->getBuffer(); // get next buffer to be written
   
            // start writing the buffer information
            while (true) {
              if (sock->ssl != nullptr)
                nwrite = (ssize_t)SSL_write(sock->ssl, &buffer[0], buffer.size());
              else
                nwrite = write(sock->fd, &buffer[0], buffer.size());
  
              // EOF Reached or Write error
              if (nwrite == -1) {
                if (errno != EAGAIN) {
                  sock->Close(-1);
                  delete sock;
                }
                break;
              
              // there is no more data to write
              } else if (nwrite < 1) {
                if (buffer.size() < 1) break;

              // write any data that wasnt written
              } else {
                buffer.erase(buffer.begin(), buffer.begin() + nwrite);
              }
            }
          }

        // no data left to write, remove write event
        } else mod(sock->fd, EPOLL_CTL_MOD, EPOLLIN | EPOLLET, sock);
      }

      // socket is ready to read
      if (event.events & EPOLLIN) {

        // if ssl and not fully connected, complete handshake
        if (sock->ssl != nullptr && !sock->connected) {
          if (sslHandshake(sock) == 0)
            sock->setConnected();
          continue;
        }

        // prepare reader
        io::Data reader;

        // start reading data
        while (true) {
          if (sock->ssl != nullptr)
            nread = (ssize_t)SSL_read(sock->ssl, rbuf, sizeof(rbuf));
          else
            nread = read(sock->fd, rbuf, sizeof(rbuf));

          // EOF reached, stop reading
          if (nread == -1) {
            if (errno != EAGAIN) {
              sock->Close(-1);
              delete sock;
            }
            break;

          // there is no more data to read, stop reading
          } else if (nread == 0) break;

          // there is still data left to read, clear and keep reading
          else reader.insert(reader.end(), rbuf, rbuf + nread);
          std::memset(rbuf, 0, sizeof(rbuf));
        }

        // emit read event if there was data collected
        if (reader.size() > 0)
          sock->performRead(reader);
      }
    }
  }

  // when io loop exits, free data
  free(events);
  close(epoll);
  SSL_CTX_free(ctx);
  return 0;
}