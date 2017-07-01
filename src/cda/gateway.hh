#pragma once

#include "rest.hh"

namespace cda {

  // Predef to avoid circular headers
  class Client;

  class Gateway {
  private:
    io::uint shards; // amount of shards spawned
    std::string url; // the base url to connect to
    std::shared_ptr<io::WebsockClient> conn; // the websocket client

  public:
    io::uint id;              // the shard id
    Client *client = nullptr; // the discord client

    /**
     * Initialize a gateway connection
     * @param {uint} id the gateway shard id
     * @param {uint} shards the amount of shards spawned
     * @param {Client} the discord client that spawned it
     */
    Gateway(io::uint id, io::uint shards, Client *c);

    /**
     * Start the gateway connection
     * @param {string} _url the base url to connect to
     */
    void start(const std::string &_url);

    /**
     * Handle incoming messages
     * @param {Frame} frame the incoming websocket frame
     */
    void handle(io::Frame& frame);

  };

}