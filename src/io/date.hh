#pragma once

#include "json.hh"

namespace io {

  class Date {
  /** Basic Date object */
  private:
    std::tm dtime;
  public:
    /** Date constructors */
    inline Date() : Date(now()) {}
    inline Date(const std::time_t _time) {
      dtime = *std::gmtime(&_time);
    }
    inline Date(const std::string &datetime) {
      static const char* format = "%Y-%m-%dT%H:%M:%SZ";
      std::istringstream ss{datetime.c_str()};
      ss >> std::get_time(&dtime, format);
    }

    /** Static function like javascript */
    inline static std::time_t now() {
      return std::time(nullptr);
    }
    
    /** Date time accessors */
    inline const int day() const {
      return dtime.tm_mday;
    }
    inline const int year() const {
      return dtime.tm_year + 1900;
    }
    inline const int month() const {
      return dtime.tm_mon + 1;
    }
    inline const int secs() const {
      return dtime.tm_sec;
    }
    inline const int mins() const {
      return dtime.tm_min;
    }
    inline const int hours() const {
      return dtime.tm_hour;
    }
    inline const std::time_t getTime() const {
      return std::mktime((std::tm*)&dtime);
    }
    inline std::string toString() {
      char t[126] = {0};
      static const char* format = "%Y-%m-%dT%H:%M:%SZ";
      std::strftime(t, sizeof(t), format, &dtime);
      return std::string(t);
    }
    
    /** Operator overloads */
    inline Date operator+(const Date& other) {
      return Date(this->getTime() + other.getTime());
    }
    inline Date operator-(const Date& other) {
      return Date(this->getTime() - other.getTime());
    }
    inline bool operator> (const Date& d2) {
      return this->getTime() > d2.getTime();
    }
    inline bool operator<= (const Date& d2) {
      return this->getTime() <= d2.getTime();
    }
    inline bool operator< (const Date& d2) {
      return this->getTime() < d2.getTime();
    }
    inline bool operator>= (const Date& d2) {
      return this->getTime() >= d2.getTime();
    }
    inline operator const char*() { return this->toString().c_str(); }
    inline friend std::ostream& operator<<(std::ostream &s, Date& d) {
      s << d.toString(); return s;
    }
  };

}