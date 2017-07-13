#pragma once

#include "gateway.hh"

namespace cda {

  class Client {
  public:
    io::Loop *loop;
    std::string token;  // the bots token
    ApiController api;  // the api handler
    io::uint numShards; // the amount of shards to spawn

    std::shared_ptr<User> user;
    std::vector<std::shared_ptr<User>> users;
    std::vector<std::shared_ptr<Guild>> guilds;

    // array of shards spawned
    std::vector<std::shared_ptr<Gateway>> shards;
    
    // deconstructors
    inline ~Client() = default;

    /**
     * Create the client
     * @param {uint} num the amount of shards to use (0 if auto)
     */
    inline Client(io::uint num = 0) {
      numShards = num;
      loop = api.loop.get();
    }

    /**
     * Start the discord client
     * @param {string} _token the bot token
     */
    int login(const std::string &_token);
  };
}