#pragma once

#include "b64.hh"

namespace io {

  /* Uri object */
  typedef struct Uri {
    int port;
    bool ssl;
    std::map<std::string, std::string> params;
    Uri(const std::string &_url) { parse(_url); }
    std::string proto, host, path, query, url; 
  private:
    void parse(const std::string &_url);
  } Uri;
  
}