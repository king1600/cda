#include "user.hh"
#include "guild.hh"
#include "../client.hh"
  
void cda::User::parse(io::json &data) {
  if (data.find("id") != data.end())
    id = data["id"];
  if (data.find("bot") != data.end())
    bot = data["bot"];
  if (data.find("verified") != data.end())
    verified = data["verified"];
  if (data.find("mfa_enabled") != data.end())
    mfa_enabled = data["mfa_enabled"];
  if (data.find("email") != data.end())
    email = data["email"];
  if (data.find("avatar") != data.end())
    avatar = data["avatar"];
  if (data.find("username") != data.end())
    username = data["username"];
  if (data.find("discriminator") != data.end())
    discrim = data["discriminator"];
}

void cda::Member::parse(io::json &data) {
  if (data.find("id") != data.end())
    id = data["id"];
  if (data.find("deaf") != data.end())
    deaf = data["deaf"];
  if (data.find("mute") != data.end())
    mute = data["mute"];
  if (data.find("joined_at") != data.end())
    joined = io::Date(data["joined_at"].get<std::string>());

  // find the roles from guild
  if (data.find("roles") != data.end()) {
    cda::snowflake rid;
    for (const io::json role_id : data["roles"]) {
      rid = role_id;
      auto it = std::find_if(
        guild->roles.begin(), guild->roles.end(),
        [rid](cda::Role& role) { return role.id == rid; });
      if (it != guild->roles.end()) roles.push_back(*it);
    }
  }

  // find the user in db
  if (data.find("user") != data.end()) {
    cda::snowflake uid = data["user"]["id"];
    user = cda::Find(uid, client->users);
    if (user.get() == nullptr) {
      user = std::make_shared<cda::User>();
      user->parse(data["user"]);
    }
  }
}