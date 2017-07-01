#include "http.hh"

/**
 * Split a string by delimiter
 * @param {string} str the string to split
 * @param {string} delim the delimiter to split by
 * @return {vector<string>} list of string tokens split
 */
std::vector<std::string> io::Split(
  std::string str, const std::string &delim)
{
  std::vector<std::string> results;
  std::size_t pos = str.find(delim);
  while (pos != std::string::npos) {
    results.push_back(str.substr(0, pos));
    str = str.substr(pos, delim.size());
    pos = str.find(delim);
  }
  if (!str.empty()) results.push_back(str);
  return results;
}

/**
 * Convert HttpRequest into HTTP String data
 * @param {HttpRequest*} req the request to dump
 * @return {Data} request if socket transmittable format
 */
static inline io::Data Dump(io::HttpRequest *req) {
  char r[4096] = {0}; // string concat buffer

  // create first line
  sprintf(r, "%s %s%s HTTP/1.1\r\n",
    req->method.c_str(),
    req->uri.path.c_str(),
    req->uri.query.c_str());

  // intialize http builder
  std::string builder(r);

  // add body content length
  if (!req->body.empty())
    if (req->headers.find("Content-Length") == req->headers.end())
      req->headers.insert(std::pair<std::string, std::string>(
        "Content-Length", std::to_string(req->body.size())));

  // add host header
  sprintf(r, "%s:%d", req->uri.host.c_str(), req->uri.port);
  if (req->headers.find("Host") == req->headers.end())
    req->headers.insert(std::pair<std::string, std::string>(
      "Host", std::string(r)));

  // write headers to http builder
  for (auto const& i : req->headers) {
    sprintf(r, "%s: %s\r\n", i.first.c_str(), i.second.c_str());
    builder += r;
  }

  // add body to build and convert into Data to be transmitted
  builder += "\r\n";
  if (!req->body.empty()) builder += req->body;
  io::Data wrapper(builder.begin(), builder.end());
  return wrapper;
}

/**
 * Perform an Async HTTP Request
 * @param {HttpRequest&} req the request object to use
 * @param {HttpCallback} callback the cb when completed
 * @return {bool} if connection successful
 */
bool io::HttpClient::Request(io::HttpRequest &req,
 io::HttpCallback callback)
{
  // http variables
  io::Socket *sock = nullptr;
  io::HttpClient *self = this;
  io::HttpRoute *route = nullptr;

  // create or get socket from cache
  bool cached = cache.find(req.uri.host) != cache.end();
  sock = cached ? 
    cache[req.uri.host].getSock() : loop->spawn(req.uri);
  if (cached) route = &cache[req.uri.host];
  if (sock == nullptr) return false;

  // if not cached, wait to be connected and write data
  io::Data buffer = Dump(&req);
  if (!cached) {
    sock->onConnect([sock, buffer]() {
      io::Data copy(buffer.begin(), buffer.end());
      sock->Write(copy);
    });

  // if cached, write the http data
  } else {
    if (cached) route->addTask(callback);
    sock->Write(buffer);
  }

  // handle reading and parsing the data
  if (!cached) {
    sock->onRead([self, sock, &req, callback]
    (io::Data &data) {
      io::HttpResponse resp;                // the response object
      std::size_t used = 0;                 // buffered parsing var
      const std::size_t size = data.size(); // size of response data
      
      // parse the data into the response
      while ((used < size) && !resp.complete())
        used += resp.feed(&data[0] + used, size - used);

      // perform callback when parsing is complete
      if (resp.complete()) {

        // check if connection is cached
        bool cached = 
          self->cache.find(req.uri.host) != self->cache.end();
  
        // add connection to cache if keep alive
        if (resp.has_header("Connection")) {
          std::string conn = resp.header("Connection");
          if (conn.find("keep-alive") != std::string::npos) {
            io::HttpRoute _route(sock);
            self->cache.insert(std::pair<std::string, io::HttpRoute>(
              req.uri.host, _route));
          
          // remove connection from cache if not keep-alive
          } else {
            if (self->cache.find(req.uri.host) != self->cache.end())
              self->cache.erase(req.uri.host);
            cached = false;
          }
        }

        // connection not cached, perform callback
        if (!cached) {
          if (self->cache.find(req.uri.host) == self->cache.end())
            delete sock;
          callback(resp);

        // connection cached, perform last callback
        } else {
          HttpRoute &route = self->cache[req.uri.host];
          if (route.hasTask()) {
            io::HttpCallback task = route.getTask();
            task(resp);
          }
        }
      }
    });

    // remove from cached when disconnected
    if (!cached) {
      sock->onClose([self, &req](int error){
        if (self->cache.find(req.uri.host) != self->cache.end())
          self->cache.erase(req.uri.host);
      });
    }
  }

  // success in establishing http client
  return true;
}