#include "uri.hh"
#include <httpxx/Url.hpp>

/**
 * Parse url into Uri object
 * @param {std::string} _url the url to parse
 */
void io::Uri::parse(const std::string &_url) {
  url = _url; // save the url
  
  // parse url and get objects
  http::Url u(url);
  proto = u.schema();
  host  = u.host();
  path  = u.path();
  query = u.query();
  ssl   = (proto == "https" || proto == "wss");

  // fix query and path
  if (path.empty()) path = "/";
  if (!query.empty()) query = "?" + query;

  // get port and query
  if (u.port().empty()) port = ssl ? 443 : 80;
  else port = std::stoi(u.port().c_str());
}