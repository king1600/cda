#pragma once

#include "rest.hh"

namespace cda {

  class Gateway {
  public:
    io::uint id;     // the shard id
    io::uint shards; // amount of shards spawned
    std::string url; // the base url to connect to
    Client *client = nullptr; // the discord client
    std::shared_ptr<io::WebsockClient> conn; // the websocket client

    int seq = -1;           // the latest packet sequence
    io::uint beatInter;     // gateway heartbeat interval
    bool acked = true;      // if heartbet was acknowledged
    bool resume = false;    // if shard should idetify via resume
    bool reconnect = true;  // if shard should reconnect
    std::string session_id; // session id for shard connection

    /**
     * Initialize a gateway connection
     * @param {uint} id the gateway shard id
     * @param {uint} shards the amount of shards spawned
     * @param {Client} the discord client that spawned it
     */
    Gateway(io::uint id, io::uint shards, Client *c);

    /**
     * Start heartbeat
     */
    void beat();

    /**
     * Identify the shard with info
     */
    void identify();

    /**
     * Send a gateway message
     * @param {uint} op the gateway opcode
     * @param {json} data the data to send
     */
    void send(io::uint op, io::json data);

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