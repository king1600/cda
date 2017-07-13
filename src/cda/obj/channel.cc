#include "channel.hh"

static inline void Parse(cda::Channel& channel, io::json& data) {
  if (data.find("id") != data.end())
    channel.id = data["id"];
  if (data.find("name") != data.end())
    channel.name = data["name"];
  if (data.find("position") != data.end())
    channel.position = data["position"];
}

void cda::TextChannel::parse(io::json &data) {
  Parse(*this, data);
  if (data.find("topic") != data.end())
    topic = data["topic"];
  if (data.find("permission_overwrites") != data.end()) {
    for (io::json &perm : data["permission_overwrites"]) {
      Overwrites overwrite;
      if (perm.find("allow") != perm.end())
        overwrite.allow = perm["allow"];
      if (perm.find("deny") != perm.end())
        overwrite.deny = perm["deny"];
      if (perm.find("id") != perm.end())
        overwrite.id = perm["id"];
      if (perm.find("type") != perm.end())
        overwrite.type = perm["type"];
      overwrites.push_back(overwrite);
    }
  }
}

void cda::VoiceChannel::parse(io::json &data) {
  Parse(*this, data);
  if (data.find("bitrate") != data.end())
    bitrate = data["bitrate"];
  if (data.find("user_limit") != data.end())
    bitrate = data["user_limit"];
  if (data.find("permission_overwrites") != data.end()) {
    for (io::json &perm : data["permission_overwrites"]) {
      Overwrites overwrite;
      if (perm.find("allow") != perm.end())
        overwrite.allow = perm["allow"];
      if (perm.find("deny") != perm.end())
        overwrite.deny = perm["deny"];
      if (perm.find("id") != perm.end())
        overwrite.id = perm["id"];
      if (perm.find("type") != perm.end())
        overwrite.type = perm["type"];
      overwrites.push_back(overwrite);
    }
  }
}