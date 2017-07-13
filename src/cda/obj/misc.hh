#pragma once

#include "item.hh"

namespace cda {

  struct StateType {
    static const unsigned char DnD = 0;
    static const unsigned char Idle = 1;
    static const unsigned char Online = 2;
    static const unsigned char Offline = 4;
    static const unsigned char Game = 6;
    static const unsigned char Stream = 8;
  };

  struct Game {
    std::string url;
    std::string name;
    unsigned char type;
    void parse(io::json &data);
  };

  struct Status {
    Game game;
    snowflake user_id;
    unsigned char state;
    void parse(io::json &data);
  };

  class Color {
  public:
    unsigned char r = 0;
    unsigned char g = 0;
    unsigned char b = 0;
    unsigned char a = 0;
    
    typedef unsigned char u;
    inline Color() {}
    inline Color(u r, u g, u b, u a) :
      r(r), g(g), b(b), a(a) {}
    const int val() const;
    inline static Color from(const int color)  {
      Color c;
      c.r = (color & 0xff000000) >> 0x18;
      c.g = (color & 0x00ff0000) >> 0x10;
      c.b = (color & 0x0000ff00) >> 0x08;
      c.a = (color & 0x000000ff) >> 0x00;
      return c;
    }
  };

  class Permissions {
  private:
    unsigned int value = 0;
  public:
    inline Permissions() = default;
    inline Permissions(unsigned int v) : value(v) {}

    inline operator unsigned int() const {
      return value;
    }
    inline operator int() const {
      return (int)value;
    }
    inline bool has(unsigned int permission) const {
      return (value & permission) > 0;
    }
    inline void add(unsigned int permission) {
      value |= permission;
    }
    inline void remove(unsigned int permission) {
      value &= ~permission;
    }

    static const unsigned int 
      CREATE_INSTANT_INVITE =  1 << 0,
      KICK_MEMBERS =  1 << 1,
      BAN_MEMBERS =  1 << 2,
      ADMINISTRATOR =  1 << 3,
      MANAGE_CHANNELS =  1 << 4,
      MANAGE_GUILD =  1 << 5,
      ADD_REACTIONS =  1 << 6,
      VIEW_AUDIT_LOG =  1 << 7,

      READ_MESSAGES =  1 << 10,
      SEND_MESSAGES =  1 << 11,
      SEND_TTS_MESSAGES =  1 << 12,
      MANAGE_MESSAGES =  1 << 13,
      EMBED_LINKS =  1 << 14,
      ATTACH_FILES =  1 << 15,
      READ_MESSAGE_HISTORY =  1 << 16,
      MENTION_EVERYONE =  1 << 17,
      USE_EXTERNAL_EMOJIS =  1 << 18,

      CONNECT =  1 << 20,
      SPEAK =  1 << 21,
      MUTE_MEMBERS =  1 << 22,
      DEAFEN_MEMBERS =  1 << 23,
      MOVE_MEMBERS =  1 << 24,
      USE_VAD =  1 << 25,

      CHANGE_NICKNAME =  1 << 26,
      MANAGE_NICKNAMES =  1 << 27,
      MANAGE_ROLES =  1 << 28,
      MANAGE_WEBHOOKS =  1 << 29,
      MANAGE_EMOJIS =  1 << 30;
  };

  class Role : public Item {
  public:
    inline Role() = default;
    inline ~Role() = default;
    Color color;
    Guild *guild;
    bool hoist;
    bool managed;
    bool mentionable;
    std::string name;
    Permissions perms;
    unsigned int position;
    void parse(io::json &data);
  };

  class Emoji : public Item {
  public:
    inline Emoji() = default;
    inline ~Emoji() = default;
    Guild *guild;
    bool managed;
    std::string name;
    bool require_colons;
    std::vector<Role> roles;
    void parse(io::json &data);
  };

  struct Overwrites {
    snowflake id;
    std::string type;
    unsigned char deny;
    unsigned char allow;
  };
}