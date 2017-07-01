#pragma once

#include "ws.hh"
#include <queue>
#include <httpxx/BufferedMessage.hpp>

namespace io {

  /** Basic string split functinon */
  std::vector<std::string> Split(
    std::string str, const std::string &delim);

  typedef http::BufferedResponse HttpResponse;
  typedef std::map<std::string, std::string> Headers;
  typedef std::function<void(HttpResponse&)> HttpCallback;

  class HttpRequest {
  public:
    Uri uri;
    Headers headers;
    std::string body;
    std::string method = "GET";
    inline HttpRequest(const std::string &url) : uri(url) {}
  };

  class HttpRoute {
  /** HTTP Route object to cache connection and tasks */
  private:
    io::Socket* conn;
    std::queue<HttpCallback> tasks;
  public:
    inline HttpRoute() = default;
    inline HttpRoute(io::Socket* sock) { conn = sock; }

    // method calls
    inline io::Socket* getSock() const { return conn; }
    inline void addTask(HttpCallback task) { tasks.push(task); }
    inline const bool hasTask() const { return !tasks.empty(); }
    inline HttpCallback getTask() { 
      HttpCallback task = std::move(tasks.front());
      tasks.pop();
      return task;
    }
  };

  class HttpClient {
  /**
   * Http Session to make async http requests
   * TODO: Cookie jar
   * TODO: Handle Redirects
   */
  private:
    Loop *loop; // inner IO loop
    std::map<std::string, HttpRoute> cache; // connection caching

  public:
    inline HttpClient(Loop *_loop) {
      loop = _loop;
    }

    /**
     * Perform http request using req object provided
     * @param {HttpRequest} req the request obejct
     * @param {HttpCallback} the response callback
     * @return {bool} if the request was successful
     */
    bool Request(HttpRequest &req, HttpCallback cb); 
  };  
}