#pragma once

#include <cmath>

#include "roo_time.h"

namespace roo_temperature {

// Representation of a temperature, internally stored as floating-point Celsius
// degrees.
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

  explicit Temperature(float tempC) : tempC_(tempC) {}

  // Using Celsius for the internal representation, so that integer C
  // temperatures (particularly, zero) behave well when compared for equality.
  float tempC_;
};

inline Temperature TempK(float tempK) { return Temperature(tempK - 273.15); }

inline Temperature TempC(float tempC) { return Temperature(tempC); }

inline Temperature TempF(float tempF) {
  return Temperature((tempF - 32.0) / 1.8);
}

inline Temperature operator+(Temperature a, Temperature b) {
  return TempC(a.AsC() + b.AsC());
}

inline Temperature operator-(Temperature a, Temperature b) {
  return TempC(a.AsC() - b.AsC());
}

class Thermometer {
 public:
  struct Reading {
    Temperature value;
    roo_time::Uptime time;
  };

  virtual ~Thermometer() {}

  // Returns the last known temperature.
  virtual Reading getLastReading() const = 0;
};

}  // namespace roo_temperature