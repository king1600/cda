#include "cda/cda.hh"

namespace discord = cda;

int main() {
  discord::Client client;
  std::string token = "";
  
  /*
  // example interval callbing
  io::Callback interval = [&loop, &interval](){
    std::cout << "hello world" << std::endl;
    loop.later(1000, [interval](){
      interval();
    });
  };
  interval();
  */

  return client.login(token);
}