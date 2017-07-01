#pragma once

#include <chrono>
#include "socket.hh"

namespace io {

  // Callback typedefs
  typedef std::function<void()> Callback;
  typedef std::chrono::steady_clock Clock;
  typedef Clock::time_point TimeStamp;
  typedef std::chrono::duration<double> Duration;

  // Timer task
  typedef struct Promise {
    long delay;
    bool cancelled;
    Callback callback;
    TimeStamp created;
  } Promise;

  class Loop {
  private:
    int epoll;                 // the internal epoll file descriptor
    bool running = false;      // the event loop state
    std::queue<Promise> tasks; // timed callbacks

  public:
    SSL_CTX *ctx; // the ssl shared client context

    /**
     * Initalize an event loop
     */
    Loop();

    /**
     * Start the io event loop
     * @return {int} for ease of use
     */
    int run();

    /**
     * Modify epoll triggers on a socket file descriptor
     * @param {int} fd the socket file descriptor
     * @param {int} op the EPOLL_CTL operation to perform
     * @param {int} flags the EPOLL flags to set
     * @param {void*} data optional data to attach
     */
    int mod(int fd, int op, int flags, void *data = nullptr);

    /**
     * Spawn a TCP Socket connection using the uri provided
     * @param {Uri} uri the uri to connect with
     * @return {Socket} the socket object if success else nullptr
     */
    io::Socket *spawn(Uri uri);

    /**
     * Create a promise to be resolved sometime later
     * @param {long} delay, the time to wait before fulfilling
     * @param {Callback} the action to fulfill
     * @return {Promise} the cancellable promise object
     */
    inline Promise later(long delay, Callback callback) {
      if (!callback)
        throw std::invalid_argument("No callback provided");
      Promise task = {delay, false, callback, Clock::now()};
      tasks.push(task);
      return task;
    }

    /** Close the event event */
    inline void quit() { running = false; }
  };
}