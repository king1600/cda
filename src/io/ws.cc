#include "ws.hh"
#include <random>
#include <bitset>

// random byte generation
static std::random_device RNG;
static std::mt19937 Rand(RNG());
static std::uniform_int_distribution<int> BitGen(1, 256);

// Prebaked Handkshake request
static const char *HANDSHAKE = 
"GET %s HTTP/1.1\r\n"
"Host: %s:%d\r\n"
"Upgrade: WebSocket\r\n"
"Connection: Upgrade\r\n"
"Sec-WebSocket-Key: %s\r\n"
"Sec-WebSocket-Version: 13\r\n"
"\r\n";

/**
 * Generate a random byte
 * (change function if neccesary)
 * @return {unsigned char} a random byte
 */
static inline unsigned char randByte() {
  return (unsigned char)(BitGen(Rand));
}

/**
 * Calculate the dumped size of a frame
 * @param {Frame*} frame the frame to calculate
 * @return {size_t} the size of the dumped frame
 */
static inline size_t FrameSize(io::Frame *frame) {
  size_t i = 2 + (frame->masked ? 4 : 0) + frame->len;
  if (frame->len >= 126 && frame->len < 65536) i += 2;
  else if (frame->len >= 0x10000) i += 8;
  return i;
}

/**
 * Parse data into websocket frame
 * @param {Frame*} frame the frame to fill with parsed info
 * @param {const char*} data the data to parse
 * @param {size_t} len the size of the data to parse
 */
static void FrameParse(io::Frame *frame, const char *data, size_t len) {
  // create variables and empty out current
  std::size_t offset = 0;
  int i, size, count = 0;
  std::memset(frame, 0, sizeof(*frame));
  
  // get frame first byte header info
  frame->fin  = (data[offset] & 0x80) != 0 ? 1 : 0;
  frame->rsv1 = (data[offset] & 0x40) != 0 ? 1 : 0;
  frame->rsv2 = (data[offset] & 0x20) != 0 ? 1 : 0;
  frame->rsv3 = (data[offset] & 0x10) != 0 ? 1 : 0;
  frame->opcode = data[offset++] & 0x0f;
  
  // get frame masked state and payload length
  frame->masked = (data[offset] & 0x80) != 0 ? 1 :0;
  size = (int)(data[offset++] & (~0x80));
  if (size == 0x7f) count = 8;
  else if (size == 0x7e) count = 2;
  if (count > 0) size = 0;
  while (count-- > 0)
    size |= (data[offset++] & 0xff) << (8 * count);
  frame->len = (std::size_t)size;
  
  // if masked, get mask from frame
  char mask[4];
  if (frame->masked)
    for (i = 0; i < 4; i++)
      mask[i] = data[offset++];
  
  // allocate frame payload
  free(frame->data);
  frame->data = (char*)std::calloc(
    size + 1, sizeof(char*));

  // get payload and mask it if neccessary
  for (i = 0; i < size; i++)
    frame->data[i] = !frame->masked ? data[offset + i]
      : data[offset + i] ^ mask[i % 4];
}

/**
 * Dump websocket frame into data to send out
 * @param {Frame*} frame the frame to dump from
 * @param {char*} out the output of the dumped frame
 */
static void FrameDump(io::Frame *frame, char *out) {
  int i;                  // iteration variables
  std::size_t offset = 0; // calculation variables
  
  // set first header byte
  out[offset] = (char)(frame->fin ? 0x80 : 0);
  out[offset] |= (frame->rsv1 ? 0x40 : 0);
  out[offset] |= (frame->rsv2 ? 0x20 : 0);
  out[offset] |= (frame->rsv3 ? 0x10 : 0);
  out[offset++] |= (char)frame->opcode;
    
  // set second header byte
  char masked = (char)(frame->masked ? 0x80 : 0);
  if (frame->len <= 125) {
    out[offset++] = (char)(masked | frame->len);
  } else if (frame->len >= 126 && frame->len < 65536) {
    out[offset++] = (char)(masked | 0x7e);
    for (i = 8; i >= 0; i -= 8)
      out[offset++] = (char)((frame->len >> i) & 0xff);
  } else {
    out[offset++] = (char)(masked | 0x7f);
    for (i = 56; i >= 0; i -= 8)
      out[offset++] = (char)((frame->len >> i) & 0xff);
  }
  
  // if frame masked, create mask
  std::vector<unsigned char> mask;
  if (frame->masked) {
    mask.reserve(4);
    for (i = 0; i < 4; i++) {
      mask[i] = randByte();
      out[offset++] = mask[i];
    }
  }
  
  // add payload data
  for (i = 0; i < (signed)frame->len; i++)
    out[offset + i] = (char)(!frame->masked ?
      frame->data[i] : (frame->data[i] ^ mask[i % 4]));
}

/**
 * Close the websocket
 * @param {int} status the close statuc code
 * @param {std::string} reason the close status reason
 */
void io::WebsockClient::Close(int status, const std::string &reason) {
  if (sock != nullptr) {
    delete sock;
  }
  close_cb(status, reason);
}

/**
 * Send data over the websocket
 * @param {std::string} data the data to send
 * @param {unsigned} opcode the opcode to use
 */
void io::WebsockClient::Send(const std::string &data, unsigned opcode) {
  // create the frame
  io::Frame frame = { 0 };
  frame.fin    = 1;
  frame.masked = 1;
  frame.opcode = opcode;
  frame.data   = (char*)data.c_str();
  frame.len    = std::strlen(frame.data);

  // convert frame to transferable data
  char buffer[FrameSize(&frame)] = { 0 };
  FrameDump(&frame, buffer);
  io::Data output(buffer, buffer + sizeof(buffer));

  // send data over socket
  if (sock != nullptr)
    sock->Write(output);
}

/**
 * Start connection with the websocket
 * @param {std::string} url the url to connect to
 */
bool io::WebsockClient::Connect(const std::string &url) {
  // attempt to connect
  io::Uri uri(url);
  sock = loop->spawn(uri);
  if (sock == nullptr) return false;

  // builders and frame variabels
  io::Data builder;
  io::Frame frame = { 0 };

  // handle coming from the websocket
  sock->onRead([this, &builder, &frame](io::Data &data) {

    // handle handshake response
    if (!this->connected) {
      std::string http(data.begin(), data.end());
      if (http.find("HTTP/1.1 101") != std::string::npos) {
        this->connected = true;
        this->connect_cb();
      } else this->Close(1005, "");

    // handle websocket frames
    } else {

      FrameParse(&frame, &data[0], data.size());

      // handle nont continuation frams
      if (frame.fin && frame.opcode != io::Opcode::CONT) {

        // close on opcode
        if (frame.opcode == io::Opcode::CLOSE) {
          this->Close(1000, "");
          return;
        }

        // combine the builder data and the received data
        io::Data merged(frame.len + builder.size());
        merged.insert(merged.begin(), &builder[0],
          &builder[0] + builder.size());
        merged.insert(merged.begin() + builder.size(),
          frame.data, frame.data + frame.len);
        frame.len = merged.size();
        memcpy(frame.data, &merged[0], frame.len);

        // perform message callback and clear builder
        this->message_cb(frame);
        builder.clear();

      // packet is continuation, add data to builder
      } else {
        builder.insert(builder.end(),
          frame.data, frame.data + frame.len);
      }
    }
  });

  // create websocket sec-key for handshake
  unsigned char key[17] = { 0 };
  for (unsigned i = 0; i < 16; i++)
    key[i] = randByte();

  // create handshake http data
  char httpHandshake[1024] = { 0 };
  sprintf(httpHandshake, HANDSHAKE, 
    (uri.path + uri.query).c_str(), 
    uri.host.c_str(), uri.port,
    (const char*)io::b64_encode(key, 16));

  // send handshake when conencted
  sock->onConnect([this, httpHandshake]() {
    this->sock->Write(httpHandshake, strlen(httpHandshake));
  });

  // return successful startup
  return true;
}