#pragma once

#include <cmath>

#include "roo_time.h"

namespace roo_temperature {

// Representation of a temperature, internally stored as floating-point Celsius
// degrees.
class Temperature {
 public:
  Temperature() : tempC_(std::nanf("")) {}

  float degKelvin() const { return tempC_ + 273.15; }
  float degCelcius() const { return tempC_; }
  float degFahrenheit() const { return tempC_ * 1.8 + 32.0; }

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
  friend Temperature Unknown();

  friend Temperature DegKelvin(float);
  friend Temperature DegCelcius(float);
  friend Temperature DegFahrenheit(float);

  explicit Temperature(float tempC) : tempC_(tempC) {}

  // Using Celsius for the internal representation, so that integer C
  // temperatures (particularly, zero) behave well when compared for equality.
  float tempC_;
};

inline Temperature Unknown() { return Temperature(); }

inline Temperature DegKelvin(float tempK) {
  return Temperature(tempK - 273.15);
}

inline Temperature DegCelcius(float tempC) { return Temperature(tempC); }

inline Temperature DegFahrenheit(float tempF) {
  return DegCelcius((tempF - 32.0) / 1.8);
}

inline Temperature operator+(Temperature a, Temperature b) {
  return DegCelcius(a.degCelcius() + b.degCelcius());
}

inline Temperature operator-(Temperature a, Temperature b) {
  return DegCelcius(a.degCelcius() - b.degCelcius());
}

class Thermometer {
 public:
  struct Reading {
    Temperature value;
    roo_time::Uptime time;
  };

  virtual ~Thermometer() = default;

  // Returns the last known temperature.
  virtual Reading readTemperature() const = 0;
};

// Reports readings or another thermometer, if they are fresher than a specified
// threshold. Otherwise, reports Unknown.
class CachingThermometer : public Thermometer {
 public:
  CachingThermometer(const Thermometer *thermometer)
      : CachingThermometer(thermometer, roo_time::Hours(10000000)) {}

  CachingThermometer(const Thermometer *thermometer,
                     roo_time::Interval staleness_threshold)
      : thermometer_(thermometer), staleness_threshold_(staleness_threshold) {}

  void setStalenessThreshold(roo_time::Interval staleness_threshold) {
    staleness_threshold_ = staleness_threshold;
  }

  roo_time::Interval stalenessThreshold() const { return staleness_threshold_; }

  Reading readTemperature() const override {
    Reading reading = thermometer_->readTemperature();
    if (reading.time + staleness_threshold_ < roo_time::Uptime::Now()) {
      reading.value = Unknown();
    }
    return reading;
  }

 private:
  const Thermometer *thermometer_;
  roo_time::Interval staleness_threshold_;
};

}  // namespace roo_temperature