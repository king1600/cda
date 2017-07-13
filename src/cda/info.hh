#pragma once
#include <string>
#include <cstdio>

#define CDA_STRING(v) CDA_STRING_HELPER(v)
#define CDA_STRING_HELPER(v) #v

#define CDA_VERSION_MAJOR 0
#define CDA_VERSION_MINOR 1
#define CDA_VERSION_PATCH 0
#define CDA_VERSION_STRING \
  CDA_STRING(CDA_VERSION_MAJOR) "." \
  CDA_STRING(CDA_VERSION_MINOR) "." \
  CDA_STRING(CDA_VERSION_PATCH)

namespace cda {

  struct Version {
    static const unsigned int Major =
      CDA_VERSION_MAJOR;
    static const unsigned int Minor =
      CDA_VERSION_MINOR;
    static const unsigned int Patch =
      CDA_VERSION_PATCH;
    static inline const std::string String() {
      return CDA_VERSION_STRING;
    }
  };

  static const std::string Lib = "cda";
  static const std::string ApiVersion = "?v=6&encoding=json";
  static const std::string Endpoint = "https://discordapp.com/api";
  static const std::string Url = "http://github.com/king1600/cda";

  struct Op {
    static const unsigned int DISPATCH           = 0;
    static const unsigned int HEARTBEAT          = 1;
    static const unsigned int IDENTIFY           = 2;
    static const unsigned int STATUS_UPDATE      = 3;
    static const unsigned int VOICE_STATE_UPDATE = 4;
    static const unsigned int VOICE_SERVER_PING  = 5;
    static const unsigned int RESUME             = 6;
    static const unsigned int RECONNECT          = 7;
    static const unsigned int GUILD_MEMBERS      = 8;
    static const unsigned int INVALID_SESSION    = 9;
    static const unsigned int HELLO              = 10;
    static const unsigned int HEARTBEAT_ACK      = 11;
  };

  static inline const std::string OSName() {
    #ifdef _WIN32
      return "win32";
    #elif _WIN64
      return "win64";
    #elif __linux__
      return "linux";
    #elif __APPLE__ || __MACH__
      return "darwin";
    #elif __unix || __unix__
      return "unix";
    #elif __FreeBSD__
      return "freebsd";
    #else
      return "other";
    #endif
  }
}