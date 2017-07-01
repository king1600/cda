#include "client.hh"

int cda::Client::login(const std::string &_token) {
  token = _token;     // save the token internally
  api.token = _token; //  save token to api controller

  // fecth the gateway url
  cda::Client *self = this;
  api.get("/gateway/bot", {}, [self](io::json &resp) {

    // get the amount of shards to use
    self->numShards = self->numShards > 0 ?
      self->numShards : resp["shards"].get<io::uint>();

    // create shard connections
    for (io::uint i = 0; i < self->numShards; i++) {
      std::shared_ptr<cda::Gateway> shard = 
        std::make_shared<cda::Gateway>(i, self->numShards, self);
      self->shards.push_back(shard);
    }

    // Start the shard connections
    std::string url = resp["url"];
    for (auto shard : self->shards)
      shard->start(url);
  });

  // start the internal event loop
  return api.loop->run();
}