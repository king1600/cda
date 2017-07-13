#include "guild.hh"
#include "user.hh"
#include "channel.hh"

void cda::Emoji::parse(io::json &data) {

}

void cda::Role::parse(io::json &data) {

}

void cda::Guild::parse(io::json &data) {
  // load basic attributes
  if (data.find("id") != data.end())
    id = data["id"];
  if (data.find("name") != data.end())
    name = data["name"];
  if (data.find("large") != data.end())
    large = data["large"];
  if (data.find("region") != data.end())
    region = data["region"];
  if (data.find("joined_at") != data.end())
    joined = io::Date(data["joined_at"].get<std::string>());
  if (data.find("mfa_level") != data.end())
    mfa_level = data["mfa_level"];
  if (data.find("member_count") != data.end())
    member_count = data["member_count"];
  if (data.find("verification_level") != data.end())
    verify_level = data["verification_level"];
  if (data.find("unavailable") != data.end())
    unavailable = data["unavailable"];
  if (data.find("explicit_content_filter") != data.end())
    explicit_filter = data["explicit_content_filter"];
  if (data.find("default_message_notifications") != data.end())
    default_notifs = data["default_message_notifications"];

  // load nullable attributes
  if (data.find("icon") != data.end())
    if (!data["icon"].is_null())
      icon = data["icon"];
  if (data.find("splash") != data.end())
    if (!data["splash"].is_null())
      splash = data["splash"];
  if (data.find("afk_channel_id") != data.end())
    if (!data["afk_channel_id"].is_null())
      afk_channel_id = data["afk_channel_id"];
  if (data.find("afk_timeout") != data.end())
    if (!data["afk_timeout"].is_null())
      afk_timeout = data["afk_timeout"];

  // load emojies
  if (data.find("emojis") != data.end()) {
    for (io::json& e : data["emojis"]) {
      cda::Emoji emoji;
      emoji.parse(e);
      emoji.guild = this;
      emojis.push_back(emoji);
    }
  }

  // load roles
  if (data.find("roles") != data.end()) {
    for (io::json& r : data["roles"]) {
      cda::Role role;
      role.parse(r);
      role.guild = this;
      roles.push_back(role);
    }
  }

  // load members
  if (data.find("members") != data.end()) {
    for (io::json &m : data["members"]) {
      std::shared_ptr<cda::Member> member 
        = std::make_shared<cda::Member>();
      member->guild = this;
      member->client = client;
      member->parse(m);
      members.push_back(member);
    }
  }

  // Get member owner
  if (data.find("owner_id") != data.end()) {
    const std::string oid = data["owner_id"];
    std::shared_ptr<cda::Member> owner = 
      cda::Find<cda::Member>(cda::toId(oid), members);
  }

  // load channels
  if (data.find("channels") != data.end()) {
    for (io::json& chan : data["channels"]) {
      io::uint type = chan["type"];
      std::shared_ptr<Channel> channel;
      if (type == cda::Channel::Type::Text)
        channel = std::make_shared<cda::TextChannel>();
      else if (type == cda::Channel::Type::Voice)
        channel = std::make_shared<cda::VoiceChannel>();
      channel->client = client;
      channel->guild = this;
      channel->parse(chan);
      channels.push_back(channel);
    }
  }
}