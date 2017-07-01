#include "events.hh"

io::Emitter::Emitter() {}
io::Emitter::~Emitter() {}

io::uint io::Emitter::add(io::uint eid, std::function<void()> cb) {
  if (!cb)
    throw std::invalid_argument("No callback given!");
  std::lock_guard<std::mutex> lock(mutex);
  uint lid = ++last;
  map.insert(std::make_pair(eid,
    std::make_shared<Listener<>>(lid, cb)));
  return lid;
}

io::uint io::Emitter::on(io::uint eid, std::function<void()> cb) {
  return add(eid, cb);
}

void io::Emitter::remove(io::uint lid) {
  std::lock_guard<std::mutex> lock(mutex);
  auto id = std::find_if(map.begin(), map.end(),
  [&](std::pair<const io::uint, std::shared_ptr<ListenerBase>> p) {
    return p.second->id == lid;
  });
  if (id != map.end())
    map.erase(id);
  else
    throw std::invalid_argument("Invalid event id");
}