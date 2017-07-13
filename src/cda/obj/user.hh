#pragma once

#include "misc.hh"

namespace cda {

  class User : public Item {
  public:
    bool bot;
    bool verified;
    bool mfa_enabled;
    io::Date created;
    std::string email;
    std::string avatar;
    std::string username;
    unsigned short discrim;
    void parse(io::json &data);
  };

  class Member : public Item {
  public:
    bool deaf;
    bool mute;
    Guild* guild;
    io::Date joined;
    std::string nick;
    std::vector<Role> roles;
    std::shared_ptr<User> user;
    void parse(io::json &data);
  };
}