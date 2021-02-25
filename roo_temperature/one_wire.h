#pragma once

#include <Arduino.h>

#include <functional>

#include "DallasTemperature.h"
#include "roo_scheduler.h"
#include "roo_temperature.h"

namespace roo_temperature {

class OneWireThermometer : public Thermometer {
 public:
  class Address {
   public:
    Address() {}
    Address(const String &address);
    Address(const char* address);

   private:
    friend class OneWireThermometer;
    friend class OneWireController;

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

  OneWireThermometer(const Address &address, Range validRange,
                     float calibrationOffset = 0, String label = "");

  OneWireThermometer(const Address &address, float calibrationOffset = 0,
                     String label = "")
      : OneWireThermometer(address, Range(TempC(-120.0), TempC(80.0)),
                           calibrationOffset, label) {}

  void setCalibrationOffset(double calibrationOffset) {
    calibrationOffset_ = calibrationOffset;
  }

  Reading getLastReading() const override { return reading_; }

  const String &label() const { return label_; }

  bool isConnected() const { return connected_; }

 private:
  friend class OneWireController;

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

  // Last correctly measured temperature, or unknown if never measured.
  Reading reading_;
};

class OneWireController {
 public:
  OneWireController(OneWire *oneWire, roo_scheduler::Scheduler *scheduler,
                    std::function<void()> callback,
                    OneWireThermometer *thermometers, int16_t thermometers_size)
      : oneWire_(oneWire),
        sensors_(oneWire_),
        scheduler_(scheduler),
        // updater_(scheduler, [this]() { update(); },
        //          roo_scheduler::Interval::FromMillis(750)),
        updater_([this]() { update(); }),
        callback_(callback),
        thermometers_(thermometers),
        thermometers_size_(thermometers_size),
        permutation_(thermometers_size, 0),
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
  OneWireThermometer *thermometers_;
  int16_t thermometers_size_;
  std::vector<int> permutation_;
  bool initialized_;
  bool requested_;
};

}  // namespace roo_temperature
