#include "gateway.hh"
#include "client.hh"
#include "info.hh"



/**
 * Initialize a websocket client as well as store client
 */
cda::Gateway::Gateway(io::uint id, io::uint shards, cda::Client *c) {
  this->id = id;
  this->client = c;
  this->shards = shards;
  this->conn = std::make_shared<io::WebsockClient>(client->loop);
}

/**
 * Handle incoming websocket messages
 * @param {Frame} frame the incoming websocket frame
 */
void cda::Gateway::handle(io::Frame &frame) {
  if (frame.data[0] != '{') return; // dont parse json!

  // extract data from frame
  io::json data = io::json::parse(frame.data);
  std::cerr << "Got Data: " << data.dump(2) << std::endl;

  // handle discord opcodes
  switch (data["op"].get<io::uint>()) {
    // TODO later: handle opcode stuff here
  }
}

/**
 * Start the gateway client
 * @param {string} url the base url to connect to
 */
void cda::Gateway::start(const std::string &_url) {
  url = _url;
  cda::Gateway *self = this;

  // handle incoming messages
  conn->onMessage([self](io::Frame &frame){
    self->handle(frame);
  });

  // respawn connection when killed
  conn->onClose([self](int status, std::string reason){
    //self->start(_url);
    std::cerr << "Shard " << self->id  << " disconnected" << std::endl;
  });

  // Attempt to connect to gateway
  std::string readUrl = url + cda::ApiVersion;
  std::cerr << "Shard " << self->id << " connecting to: "
    << readUrl << std::endl;
    
  if (!conn->Connect(readUrl)) {
    char err[256] = {0};
    std::sprintf(err, "Shard %d failed to connect to gateway", id);
    throw std::runtime_error(err);
  }
}