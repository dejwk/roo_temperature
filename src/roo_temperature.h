#pragma once

#include <cmath>

#include "roo_flags.h"
#include "roo_logging.h"
#include "roo_time.h"

#if defined(ESP32) || defined(ESP8266) || defined(__linux__)
#include <string>
#endif

#if defined(ARDUINO)
#include <Arduino.h>
#endif

ROO_DECLARE_FLAG(char, roo_temperature_default_unit);

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

#if defined(ESP32) || defined(ESP8266) || defined(__linux__)
  // Returns the string representation of the temperature, using the unit
  // defined by the 'roo_temperature_default_unit' flag.
  std::string asString() const;
#endif

#if defined(ARDUINO)
  String asArduinoString() const;
#endif

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

roo_logging::Stream &operator<<(roo_logging::Stream &os, const Temperature &t);

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

}  // namespace roo_temperature