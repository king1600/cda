#include "gateway.hh"
#include "client.hh"
#include "info.hh"

/** packet event handler declaration */
void handleEvent(cda::Gateway* shard, io::json &packet);

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
 * Send a gateway message
 * @param {uint} op the gateway opcode
 * @param {json} data the data to send
 */
void cda::Gateway::send(io::uint op, io::json data) {
  io::json packet = {{"op", op}, {"d", data}};
  if (conn->isConnected())
    conn->Send(packet.dump());
}

/**
 * Attempt a gateway connection
 * @param {Gateway} shard the gateway thats trying to connect
 */
static inline void Connect(cda::Gateway* shard) {
  std::cerr << "[cda] Shard " << shard->id << " connecting to: "
    << (shard->url + cda::ApiVersion) << std::endl;
  if (!shard->conn->Connect(shard->url + cda::ApiVersion)) {
    std::cerr << "[cda] Shard " << shard->id << 
      " failed to connect to gateway!" << std::endl;
    std::cerr << "[cda] Shard " << shard->id << 
      " retrying in 5 seconds" << std::endl;
    shard->client->loop->later(5000, [shard](){
      Connect(shard);
    });
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
  conn->onClose([self, _url](int status, std::string reason){
    std::cerr << "Shard " << self->id  << " disconnected" << std::endl;
    self->client->loop->later(1000, [self](){
      Connect(self);
    });
  });
  
  // start connection
  Connect(this);
}

/**
 * Start heartbeat
 */
void cda::Gateway::beat() {
  // check if connected
  if (!conn->isConnected()) return;

  // exit if heartbeat was not acknowledged
  if (!acked) {
    resume = true;
    conn->Close(1000, "");
    return;
  }

  // create data to send in heartbeat
  io::json data;
  if (seq >= 0)
    data = io::json::parse(std::to_string(seq));
  else
    data = io::json::parse("null");
  
  // send data and reset ack
  send(cda::Op::HEARTBEAT, data);
  acked = false;

  // continue heartbeat
  cda::Gateway *self = this;
  client->loop->later(beatInter, [self](){
    self->beat();
  });
}

/**
 * Identify the shard with info
 */
void cda::Gateway::identify() {
  io::uint op;
  io::json data;
  
  // create inital identify packet
  if (!resume) {
    op = cda::Op::IDENTIFY;
    data = {
      {"token", client->token},
      {"compress", false},
      {"large_threshold", 250},
      {"shard", io::json::array({id, shards})},
      {"properties", {
        {"$os", cda::OSName()},
        {"$browser", cda::Lib},
        {"$device", cda::Lib},
        {"$referrer", ""},
        {"$referring_domain", ""}
      }}
    };

  // create resume packet
  } else {
    op = cda::Op::RESUME;
    data = {
      {"seq", seq},
      {"token", client->token},
      {"session_id", session_id}
    };
  }

  // send the identification data
  send(op, data);
}

/**
 * Handle incoming websocket messages
 * @param {Frame} frame the incoming websocket frame
 */
void cda::Gateway::handle(io::Frame &frame) {
  // dont parse json!
  if (frame.data[0] != '{') return;
  char end = frame.data[std::strlen(frame.data)-1];
  if (end != '}') return;

  // extract data from frame
  io::json data = io::json::parse(frame.data);
  if (data.find("s") != data.end())
    if (!data["s"].is_null())
      seq = data["s"].get<io::uint>();
  std::cerr << data.dump(2) << std::endl;

  // handle discord opcodes
  switch (data["op"].get<io::uint>()) {

    // handle hello packets
    case cda::Op::HELLO: {
      beatInter = data["d"]["heartbeat_interval"];
      beatInter -= 100; // go under the limit
      identify();
      break;
    }

    // handle gateway events
    case cda::Op::DISPATCH: {
      handleEvent(this, data);
      break;
    }

    // handle heartbeat acks
    case cda::Op::HEARTBEAT_ACK: {
      acked = true;
      break;
    }

    // handle invalid sessions
    case cda::Op::INVALID_SESSION: {
      resume = false;
      cda::Gateway *self = this;
      client->loop->later(5000, [self](){
        self->conn->Close(1011, "");
      });
      break;
    }

    // handle reconnect
    case cda::Op::RECONNECT: {
      break;
    }
  }
}

/**
 * Handle DISPATCH event packets for gateway
 * @param {Gateway} shard the gateway shard to handle from
 * @param {json} packet the packet to handle or use
 */
void handleEvent(cda::Gateway *shard, io::json &packet) {
  std::string event = packet["t"];
  io::json &data    = packet["d"];
 
  if (event == "RESUMED") {
    shard->resume = false;
    shard->beat();

  } else if (event == "READY") {
    shard->beat();
    for (io::json &guild : data["guilds"]) {
      
    }
  }
}