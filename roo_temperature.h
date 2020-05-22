#pragma once

#include <Arduino.h>
#include <DallasTemperature.h>

#include <functional>

#include "roo_scheduler.h"

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
  class Address {
   public:
    Address() {}
    Address(const String &address);

   private:
    friend class Thermometer;
    friend class ThemometerController;

    DeviceAddress address_;
  };

  class Range {
   public:
    Range(Temperature min, Temperature max) : min_(min), max_(max) {}
    const Temperature &getMin() const { return min_; }
    const Temperature &getMax() const { return max_; }

   private:
    Temperature min_;
    Temperature max_;
  };

  Thermometer(const Address &address, Range validRange,
              float calibrationOffset = 0, String label = "");

  Thermometer(const String &address, float calibrationOffset = 0,
              String label = "")
      : Thermometer(address, Range(TempC(-120.0), TempC(80.0)),
                    calibrationOffset, label) {}

  void setCalibrationOffset(double calibrationOffset) {
    calibrationOffset_ = calibrationOffset;
  }

  Temperature getLastReading() const { return temp_; }

  roo_time::Uptime getLastReadingTime() const { return last_conversion_; }

  const String &label() const { return label_; }

  bool isConnected() const { return connected_; }

 private:
  friend class ThermometerController;

  bool isWithinValidRange(Temperature temp) {
    return temp >= validRange_.getMin() && temp <= validRange_.getMax();
  }

  bool requestConversion(DallasTemperature *sensors);

  // Called by the controller when conversion is finished, to read off
  // the temperatures from devices.
  bool update(DallasTemperature *sensors);

  const DeviceAddress &address() const { return address_.address_; }

  Address address_;
  Range validRange_;
  String label_;
  float calibrationOffset_;
  bool connected_;
  bool requested_;

  // Last measured temperature, or unknown if never measured, or when the
  // last reading was erroneous.
  Temperature temp_;

  // The timestamp of the last completed conversion. Zero if no conversion
  // has been ever attempted.
  roo_time::Uptime last_conversion_;
};

class ThermometerController {
 public:
  ThermometerController(OneWire *oneWire, roo_scheduler::Scheduler *scheduler,
                        std::function<void()> callback,
                        Thermometer *thermometers, int16_t thermometers_size)
      : oneWire_(oneWire),
        sensors_(oneWire_),
        scheduler_(scheduler),
        // updater_(scheduler, [this]() { update(); },
        //          roo_scheduler::Interval::FromMillis(750)),
        updater_([this]() { update(); }),
        callback_(callback),
        thermometers_(thermometers),
        thermometers_size_(thermometers_size),
        permutation_(0, thermometers_size),
        initialized_(false),
        requested_(false) {
    for (int i = 0; i < thermometers_size; i++) {
      permutation_[i] = i;
    }
  }

  // const Thermometer &thermometer(int index) const {
  //   return thermometers_[index];
  // }

  int deviceCount() const { return sensors_.getDeviceCount(); }

  void locateThermometers();

  void setup(int resolution);

  void requestConversion() {
    if (requested_) return;
    requested_ = true;
    Serial.println("Requesting temperature conversion");
    // Randomize the order so that if the line is flaky, we distribute it among
    // the sensors.
    int idx[thermometers_size_];
    for (int i = 0; i < thermometers_size_; i++) idx[i] = i;
    std::random_shuffle(&permutation_[0], &permutation_[thermometers_size_]);
    for (int j = 0; j < 5; j++) {
      bool all = true;
      for (int16_t i = 0; i < thermometers_size_; i++) {
        if (!thermometers_[permutation_[i]].requestConversion(&sensors_))
          all = false;
      }
      if (all) break;
    }
    scheduler_->scheduleAfter(
        &updater_, roo_time::Millis(sensors_.millisToWaitForConversion(
                       sensors_.getResolution())));
  }

 private:
  void update() {
    requested_ = false;
    int idx[thermometers_size_];
    for (int i = 0; i < thermometers_size_; i++) idx[i] = i;
    Serial.println("Temperature conversion completed");
    for (int16_t i = 0; i < thermometers_size_; ++i) {
      thermometers_[permutation_[i]].update(&sensors_);
    }
    callback_();
  }

  OneWire *oneWire_;
  mutable DallasTemperature sensors_;
  roo_scheduler::Scheduler *scheduler_;
  roo_scheduler::Task updater_;
  std::function<void()> callback_;

  // arrays to hold device address
  Thermometer *thermometers_;
  int16_t thermometers_size_;
  std::vector<int> permutation_;
  bool initialized_;
  bool requested_;
};

}  // namespace roo_temperature