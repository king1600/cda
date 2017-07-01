#pragma once

#include "loop.hh"

namespace io {

  /* Websocket OpCodes */
  class Opcode {
  public:
    static const unsigned CONT  = 0x00;
    static const unsigned TEXT  = 0x01;
    static const unsigned BIN   = 0x02;
    static const unsigned CLOSE = 0x08;
    static const unsigned PING  = 0x09;
    static const unsigned PONG  = 0x0a;
  };

  /* Websocket Frame container */
  typedef struct Frame {
    unsigned fin;    // 0 or 1 (bool) if frame fin
    unsigned masked; // 0 or 1 (bool) if frame masked
    unsigned opcode;  // 0 - 10 (int) frame opcode
    
    // reserved bit values
    unsigned rsv1;
    unsigned rsv2;
    unsigned rsv3;
    
    char *data; // frame payload data
    size_t len; // frame payload length
  } Frame;

  class WebsockClient {
  /**
   * Websocket client class
   */
  private:
    Loop *loop;             // the internal event loop
    Socket *sock;           // the internal socket object
    bool connected = false; // websocket connection state

    // websocket callbacks
    std::function<void()> connect_cb;
    std::function<void(Frame&)> message_cb;
    std::function<void(int, std::string)> close_cb;
  
  public:
    // close the websocket when client is killed
    inline ~WebsockClient() {
      if (sock != nullptr)
        delete sock;
      sock = nullptr;
    }

    // initialize the websocket client
    inline WebsockClient(Loop *_loop) {
      loop = _loop;
      onConnect([](){});
      onMessage([](Frame &frame){});
      onClose([](int code, std::string reason){});
    }

    /**
     * Connect to a websocket server
     * @param {std::string} url the url to connect to
     * @return {bool} if connection was successful
     */
    bool Connect(const std::string &url);

    /**
     * Close the websocket connection
     * @param {int} status the websocket opcode status
     * @param {std::string} reason the websocket close reason
     */
    void Close(int status, const std::string &reason);

    /**
     * Send a message over the websocket
     * @param {std::string} data the data to send
     * @param {unsigned} opcode the opcode to use
     */
    void Send(const std::string &data,
      unsigned opode = Opcode::TEXT);

    // bind connection callback
    inline void onConnect(std::function<void()> cb) {
      connect_cb = cb;
    }

    // bind text or binary frame callback
    inline void onMessage(std::function<void(Frame&)> cb) {
      message_cb = cb;
    }

    // bind close callback
    inline void onClose(std::function<void(int, std::string)> cb) {
      close_cb = cb;
    }
  };
}