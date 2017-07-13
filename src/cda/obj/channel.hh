#pragma once

#include "misc.hh"

namespace cda {

  class Channel : public Item {
  public:
    inline Channel() = default;
    inline ~Channel() = default;
    struct Type {
      static const io::uint Text  = 0;
      static const io::uint DM    = 1;
      static const io::uint Voice = 2;
    };

    Guild *guild;
    std::string name;
    io::uint position = 0;
    io::uint type = Type::Text;
    virtual void parse(io::json& data) {}
  };

  class DMChannel : public Channel {
  public:
    std::vector<std::shared_ptr<User>> recipients;
  };

  class TextChannel : public Channel {
  public:
    inline TextChannel() = default;
    inline ~TextChannel() = default;
    std::string topic;
    std::vector<Overwrites> overwrites;
    void parse(io::json &data);
  };

  class VoiceChannel : public Channel {
  public:
    inline VoiceChannel() = default;
    inline ~VoiceChannel() = default;
    io::uint bitrate = 0;
    io::uint user_limit = 0;
    std::vector<Overwrites> overwrites;
    void parse(io::json &data);
  };
}