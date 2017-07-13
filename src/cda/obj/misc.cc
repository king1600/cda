#include "misc.hh"

const int cda::Color::val() const {
  int color = 0;
  color |= (r & 0xff) << 24;
  color |= (g & 0xff) << 16;
  color |= (b & 0xff) << 8;
  color |= (a & 0xff);
  return color;
}

void cda::Game::parse(io::json &data) {
  if (data.find("name") != data.end())
    name = data["name"];
  if (data.find("type") != data.end())
    type = data["type"];
  if (data.find("url") != data.end())
    type = data["url"];
}

void cda::Status::parse(io::json &data) {
  if (data.find("user") != data.end())
    if (data["user"].find("id") != data["user"].end())
      user_id = data["user"]["id"];
  if (data.find("game") != data.end())
    if (!data["game"].is_null())
      game.parse(data["game"]);
  if (data.find("status") != data.end()) {
    const std::string status = data["status"];
    if (status == "idle")
      state = cda::StateType::Idle;
    else if (status == "dnd")
      state = cda::StateType::DnD;
    else if (status == "online")
      state = cda::StateType::Online;
    else if (status == "offline")
      state = cda::StateType::Offline;
  }
}