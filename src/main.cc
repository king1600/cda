#include "cda/cda.hh"

namespace discord = cda;

int main() {
  discord::Client client;
  std::string token = "";

  return client.login(token);
}