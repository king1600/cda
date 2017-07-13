#pragma once

#include "date.hh"
#include <mutex>
#include <list>

/******************************************************/
/* https://gist.github.com/rioki/1290004d7505380f2b1d */
/******************************************************/

namespace io {

  // small extras
  typedef unsigned int uint;

  class Emitter {
  public:
    Emitter();
    ~Emitter();

    void remove(uint id);
    uint on(uint id, std::function<void()> cb);
    uint add(uint id, std::function<void()> cb);

    template <typename ...Args>
    uint add(uint id, std::function<void(Args...)> cb);

    template <typename ...Args>
    uint on(uint id, std::function<void(Args...)> cb);

    template <typename ...Args>
    void emit(uint id, Args... args);

  private:

    struct ListenerBase {
      uint id;
      ListenerBase() {}
      ListenerBase(uint i) : id(i) {}
      virtual ~ListenerBase() {}
    };

    template <typename ...Args>
    struct Listener : public ListenerBase {
      Listener() {}
      std::function<void(Args...)> cb;
      Listener(uint i, std::function<void(Args...)> c)
        : ListenerBase(i), cb(c) {}
    };

    Emitter(const Emitter&) = delete;
    const Emitter& operator= (const Emitter&) = delete;

    uint last;
    std::mutex mutex;
    std::multimap<uint, std::shared_ptr<ListenerBase>> map;
  };

}

template <typename ...Args>
uint io::Emitter::add(uint eid, std::function<void(Args...)> cb) {
  if (!cb)
    throw std::invalid_argument("No callback given!");
  std::lock_guard<std::mutex> lock(mutex);
  uint lid = ++last;
  map.insert(std::make_pair(eid,
    std::make_shared<Listener<Args...>>(lid, cb)));
  return lid;
}

template <typename ...Args>
uint io::Emitter::on(uint eid, std::function<void(Args...)> cb) {
  return add(eid, cb);
}

template <typename ...Args>
void io::Emitter::emit(uint eid, Args... args) {
  std::list<std::shared_ptr<Listener<Args...>>> handlers;
  {
    std::lock_guard<std::mutex> lock(mutex);
    auto range = map.equal_range(eid);
    handlers.resize(std::distance(range.first, range.second));
    std::transform(range.first, range.second, handlers.begin(),
    [](std::pair<const uint, std::shared_ptr<ListenerBase>> p){
      auto lid = std::dynamic_pointer_cast<Listener<Args...>>(p.second);
      if (lid) return lid;
      throw std::logic_error("Invalid event signature");
    });
  }
  for (auto &handler : handlers)
    handler->cb(args...);
}