#pragma once

#include "roo_time.h"

namespace roo_temperature {

class Temperature {
 public:
  Temperature() : tempC_(std::nanf("")) {}

  float AsK() const { return tempC_ + 273.15; }
  float AsC() const { return tempC_; }
  float AsF() const { return tempC_ * 1.8 + 32.0; }

  bool isUnknown() const { return std::isnan(tempC_); }

  bool operator<(const Temperature &other) const {
    return tempC_ < other.tempC_;
  }

  bool operator==(const Temperature &other) const {
    return tempC_ == other.tempC_;
  }

  bool operator>(const Temperature &other) const {
    return other.tempC_ < tempC_;
  }

  bool operator<=(const Temperature &other) const {
    return !(other.tempC_ < tempC_);
  }

  bool operator>=(const Temperature &other) const {
    return !(tempC_ < other.tempC_);
  }

  bool operator!=(const Temperature &other) const {
    return !(tempC_ == other.tempC_);
  }

 private:
  friend Temperature TempK(float);
  friend Temperature TempC(float);
  friend Temperature TempF(float);

  Temperature(float tempC) : tempC_(tempC) {}

  // Using Celsius for the internal representation, so that integer C
  // temperatures (particularly, zero) behave well when compared for equality.
  float tempC_;
};

inline Temperature TempK(float tempK) { return Temperature(tempK - 273.15); }

inline Temperature TempC(float tempC) { return Temperature(tempC); }

inline Temperature TempF(float tempF) {
  return Temperature((tempF - 32.0) / 1.8);
}

class Thermometer {
 public:
  virtual ~Thermometer() {}

  // Returns the last known temperature.
  virtual Temperature getLastReading() const = 0;

  // Returns the uptime as of the last known temperature.
  virtual roo_time::Uptime getLastReadingTime() const = 0;
};

}  // namespace roo_temperature