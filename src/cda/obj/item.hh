#pragma once

#include "../../io/io.hh"

namespace cda {

  // discord id system
  typedef uint64_t snowflake;
  static const snowflake EPOCH = 1420070400000;
  static inline const snowflake toId(const std::string &str) {
    snowflake result;
    std::istringstream stream(str.c_str());
    if (!(stream >> result)) return 0;
    return result;
  }

  // basic discord object
  class Client;
  class Item {
  public:
    Client* client;
    snowflake id = 0;
    inline Item() {}
    inline Item(snowflake id) : id(id) {}

    // compare using id
    inline virtual bool operator!=(const Item& other) {
      return this->id != other.id;
    }
    inline virtual bool operator==(const Item& other) {
      return this->id == other.id;
    }
  };

  // predefinitions
  class User;
  class Role;
  class Guild;
  class Emoji;
  class Member;
  class Channel;
  class Message;
  class Overwrites;
  class TextChannel;
  class VoiceChannel;

  // helper functions
  template <typename T>
  std::shared_ptr<T> Find
  (snowflake id, std::vector<std::shared_ptr<T>>& items) {
    auto it = std::find_if(items.begin(), items.end(),
    [id](const std::shared_ptr<Item>& object) {
      return object->id == id;
    });
    if (it == items.end()) return nullptr;
    return (std::shared_ptr<T>)(*it);
  }
}