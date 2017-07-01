#pragma once

#include "http.hh"

namespace io {
  using json = nlohmann::json;

  template <typename ...Args>
  std::string format(const std::string &f_str, Args ...args) {
    std::size_t size = std::snprintf(nullptr, 
      0, f_str.c_str(), args...) + 1;
    std::unique_ptr<char[]> buf(new char[size]);
    std::snprintf(buf.get(), size, f_str.c_str(), args...);
    return std::string(buf.get(), buf.get() + size - 1);
  }
}