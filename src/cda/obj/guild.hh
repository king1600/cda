#pragma once

#include "misc.hh"

namespace cda {

  class Guild : public Item {
  public:
    io::Date joined;
    std::string icon;
    std::string name;
    std::string splash;

    void parse(io::json &data);
    inline Guild(snowflake id, Client *client) : Item(id) {
      this->client = client;
      unavailable  = true;
    }
    
    bool large;
    bool unavailable;
    int mfa_level = 0;
    std::string region;
    int verify_level = 0;
    int default_notifs = 0;
    int explicit_filter = 0;

    io::uint afk_timeout = 0;
    snowflake afk_channel_id = 0;
    //std::vector<> voice_states;

    io::uint member_count = 0;
    std::vector<Role> roles;
    std::vector<Emoji> emojis;
    //std::vector<> features;

    std::shared_ptr<Member> owner;
    std::vector<std::shared_ptr<Member>> members;
    std::vector<std::shared_ptr<Channel>> channels;
  };

}