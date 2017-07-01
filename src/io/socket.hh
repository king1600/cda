#pragma once

#include <queue>
#include "uri.hh"
#include <openssl/ssl.h>

namespace io {

  // short alias for byte array
  typedef std::vector<char> Data;

  /**
   * DNS Resolve a hostname with the port
   * @param {const char*} host the hostname
   * @param {int} port the port to use as service
   * @param {char*} out the resolved ipv4 address output
   */
  int Resolve(const char *host, int port, char *out);

  class Loop;
  class Socket {
  /** Asynchronous socket object */
  private:
    bool closed = false;                // socket fd state
    std::deque<Data> writeQueue;        // data to be written out
    std::function<void()> connect_cb;   // connect callback
    std::function<void(Data&)> read_cb; // data read callback
    std::function<void(int)> close_cb;  // close connection callback
  
  public:
    int fd;                 // the socket file descriptor
    SSL *ssl = nullptr;     // the sll object for ssl connections
    Loop *loop = nullptr;   // the internal event loop
    bool connected = false; // socket connection state

    /**
     * Initialize the socket
     * @param {int} _fd the file descriptor to use
     * @param {Loop} _loop the internal event loop to store
     */
    inline Socket(int _fd, Loop *_loop) {
      fd = _fd;
      loop = _loop;
      onClose([](int t){});
      onRead([](Data t){});
      onConnect([](){});
    }

    // close on deletion/deconstruction
    inline ~Socket() { Close(); }

    /**
     * Close the socket connection
     * @param {int} err the error code when closing
     */
    void Close(int err = 0);

    // perform the data read callback function
    inline void performRead(Data &data) {
      read_cb(data);
    }

    /**
     * set socket into connected state
     */
    void setConnected();

    /**
     * Send data into write queue to be written
     * @param {Data&} data the buffer to enqueue
     */
    void Write(Data &data);

    /**
     * Send a string to the write queue
     * @param {std::string} data the data to enqueue
     */
    inline void Write(const std::string &data) {
      Write(data.c_str(), data.length());
    }

    /**
     * Send a c-string to the write queue
     * @param {const char*} data the raw char pointer data
     * @param {size_t} len the size of the pointer data
     */
    inline void Write(const char *data, std::size_t len) {
      Data wrapper(len);
      std::memcpy(&wrapper[0], data, len);
      Write(wrapper);
    }

    /**
     * Check if write queue has pending data
     * @return {bool} if write queu has remaining data
     */
    inline const bool hasBuffer() const {
      return !writeQueue.empty();
    }

    /**
     * Fetch a pending buffer from the write queue
     * @return {Data} the pending buffer
     */
    inline Data getBuffer() {
      Data buffer = std::move(writeQueue.back());
      writeQueue.pop_back();
      return buffer;
    }

    // bind connection callback
    inline void onConnect(std::function<void()> cb) {
      connect_cb = cb;
    }

    // bind data real callback
    inline void onRead(std::function<void(Data&)> cb) {
      read_cb = cb;
    }

    // bind close callback
    inline void onClose(std::function<void(int)> cb) {
      close_cb = cb;
    }
  };
}