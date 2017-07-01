#include "rest.hh"
#include "info.hh"

/**
 * Perform Discord API Request
 * @param {string} method the HTTP Method to perform
 * @param {string} endpoint the discord endpoint to request
 * @param {json} body the json body of the request
 * @param {Callback} the json callback to the request
 * @return {bool} if the request was successful
 */
bool cda::ApiController::request(const std::string &method,
  const std::string &endpoint, io::json body, cda::ApiCallback callback)
{
  // create Http request
  io::HttpRequest req(cda::Endpoint + endpoint + cda::ApiVersion);

  // add headers
  char userAgent[126] = { 0 };
  std::sprintf(userAgent, "DiscordBot (%s, %s)",
    cda::Url.c_str(), cda::Version::String().c_str());
  req.headers.insert(std::pair<std::string, std::string>(
    "Authorization", ("Bot " + token)));
  req.headers.insert(std::pair<std::string, std::string>(
    "User-Agent", userAgent));

  // Get http body
  if (!body.empty()) {
    req.headers.insert(std::pair<std::string, std::string>(
      "Content-Type", "application/json"));
    req.body = body.dump();
  }

  // Perform request and return result
  cda::ApiController *self = this;
  return http->Request(req, 
  [self, method, endpoint, callback](io::HttpResponse &resp) {
    
    // Extract http response info
    io::json body = io::json::parse(resp.body());
    const int status = resp.status();

    // Do basic http rate limiting
    if (status == 429) {
      long delay = body["retry_after"];
      self->loop->later(delay, 
      [self, &method, &endpoint, &body, callback](){
        self->request(method, endpoint, body, callback);
      });

    // Perform http callbac
    } else{
      if (status >= 200 && status < 300)
        callback(body);
    }
    
  });
}