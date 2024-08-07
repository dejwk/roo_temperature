#pragma once

#include <cmath>

#include "roo_time.h"

namespace roo_temperature {

// Representation of a temperature, internally stored as floating-point Celsius
// degrees.
class Temperature {
 public:
  // Creates a temperature object representing an 'unknown' temperature.
  Temperature() : tempC_(std::nanf("")) {}

  // Returns the temperature in degrees Celcius.
  float degCelcius() const { return tempC_; }

  // Returns the temperature in degrees Kelvin.
  float degKelvin() const { return tempC_ + 273.15; }

  // Returns the temperature in degrees Fahrenheit.
  float degFahrenheit() const { return tempC_ * 1.8 + 32.0; }

  // Returns whether the object represents an unknown temperature.
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

  friend Temperature DegCelcius(float);
  friend Temperature DegKelvin(float);
  friend Temperature DegFahrenheit(float);

  explicit Temperature(float tempC) : tempC_(tempC) {}

  // Using Celsius for the internal representation, so that integer C
  // temperatures (particularly, zero) behave well when compared for equality.
  float tempC_;
};
// Returns a temperature object representing an unknown temperature.
inline Temperature Unknown() { return Temperature(); }

// Returns a temperature object equivalent to the specified temperature
// expressed in Celcius degrees.
inline Temperature DegCelcius(float tempC) { return Temperature(tempC); }

// Returns a temperature object equivalent to the specified temperature
// expressed in Kelvin degrees.
//
// Due to floating-point rounding errors, and since the temperature is
// internally stored in Celcius degrees, generally,
// DegKelvin(x).degKelvin() != x.
inline Temperature DegKelvin(float tempK) {
  return Temperature(tempK - 273.15);
}

// Returns a temperature object approximately equal to the specified temperature
// expressed in Fahrenheit degrees.
//
// Due to floating-point rounding errors, and since the temperature is
// internally stored in Celcius degrees, generally,
// DegFahrenheit(x).degFahrenheit() != x.
inline Temperature DegFahrenheit(float tempF) {
  return DegCelcius((tempF - 32.0) / 1.8);
}

inline Temperature operator+(Temperature a, Temperature b) {
  return DegCelcius(a.degCelcius() + b.degCelcius());
}

inline Temperature operator-(Temperature a, Temperature b) {
  return DegCelcius(a.degCelcius() - b.degCelcius());
}

// An abstraction of a termometer, i.e., a device that can report temperature.
class Thermometer {
 public:
  // Captures a temperature measured at a specific time.
  struct Reading {
    Temperature value;
    roo_time::Uptime time;
  };

  virtual ~Thermometer() = default;

  // Returns the latest available temperature reading.
  virtual Reading readTemperature() const = 0;
};

// Reports readings of another thermometer, if they are fresher than a specified
// expiration threshold. Otherwise, reports Unknown.
class ExpiringThermometer : public Thermometer {
 public:
  ExpiringThermometer(const Thermometer *thermometer)
      : ExpiringThermometer(thermometer, roo_time::Hours(10000000)) {}

  ExpiringThermometer(const Thermometer *thermometer,
                      roo_time::Interval expiration)
      : thermometer_(thermometer), expiration_(expiration) {}

  void setExpiration(roo_time::Interval expiration) {
    expiration_ = expiration;
  }

  roo_time::Interval expiration() const { return expiration_; }

  Reading readTemperature() const override {
    Reading reading = thermometer_->readTemperature();
    if (reading.time + expiration_ < roo_time::Uptime::Now()) {
      reading.value = Unknown();
    }
    return reading;
  }

 private:
  const Thermometer *thermometer_;
  roo_time::Interval expiration_;
};

// Convenience function that reports the temperature reading of the specified
// thermometer if it is fresher than the specified expiration threshold, and
// Unknown otherwise. Use this function if stale thermometer readings (e.g. due
// to thermometers disconnected from the bus) shouldn't be used.
inline Thermometer::Reading ReadExpiringTemperature(
    const Thermometer &t, roo_time::Interval expiration) {
  return ExpiringThermometer(&t, expiration).readTemperature();
}

}  // namespace roo_temperature