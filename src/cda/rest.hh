#pragma once

#include "obj/objects.hh"

namespace cda {

  typedef std::function<void(io::json&)> ApiCallback;
  static const io::json Jempty = io::json::parse("{}");
  static const ApiCallback DefaultCallack = [](io::json &j){};

  class ApiController {
  public:
    std::string token;
    std::shared_ptr<io::Loop> loop;
    std::shared_ptr<io::HttpClient> http;

    // create the IO objects
    inline ApiController() {
      loop = std::make_shared<io::Loop>();
      http = std::make_shared<io::HttpClient>(loop.get());
    }

    /**
      * Perform Discord API Request
      * @param {string} method the HTTP Method to perform
      * @param {string} endpoint the discord endpoint to request
      * @param {json} body the json body of the request
      * @param {Callback} the json callback to the request
      * @return {bool} if the request was successful
      */
    bool request(
      const std::string &method,             // the http method
      const std::string &endpoint,           // the discord endpoint
      io::json data = Jempty,                // optional data
      ApiCallback callback = DefaultCallack  // the result callback
    );

    /**
     * Perform a Discord GET Request 
     */
    inline bool get(const std::string &endpoint,
      io::json data = Jempty, ApiCallback callback = DefaultCallack) {
      return request("GET", endpoint, data, callback);
    }

    /**
     * Perform a Discord POST Request 
     */
    inline bool post(const std::string &endpoint,
      io::json data = Jempty, ApiCallback callback = DefaultCallack) {
      return request("POST", endpoint, data, callback);
    }

    /**
     * Perform a Discord PUT Request 
     */
    inline bool put(const std::string &endpoint,
      io::json data = Jempty, ApiCallback callback = DefaultCallack) {
      return request("PUT", endpoint, data, callback);
    }

    /**
     * Perform a Discord DELETE Request 
     */
    inline bool del(const std::string &endpoint,
      io::json data = Jempty, ApiCallback callback = DefaultCallack) {
      return request("DELETE", endpoint, data, callback);
    }

    /**
     * Perform a Discord PATCH Request 
     */
    inline bool patch(const std::string &endpoint,
      io::json data = Jempty, ApiCallback callback = DefaultCallack) {
      return request("PATCH", endpoint, data, callback);
    }
  };
}