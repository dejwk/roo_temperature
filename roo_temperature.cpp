#include "roo_temperature.h"

namespace roo_temperature {

static const unsigned long kMaxDisconnectedMs = 5 * 1000;

// function to print a device address
static void printAddress(const DeviceAddress &deviceAddress) {
  for (uint8_t i = 0; i < 8; i++) {
    if (deviceAddress[i] < 16) Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
  }
}

inline static uint8_t ParseHexNibble(const char **in) {
  char c;
  do {
    c = *(*in)++;
  } while (c == ' ');
  if (c >= '0' && c <= '9') {
    return c - '0';
  } else if (c >= 'A' && c <= 'F') {
    return c - 'A' + 10;
  } else if (c >= 'a' && c <= 'f') {
    return c - 'a' + 10;
  } else {
    Serial.print("Error parsing hex address; encountered illegal character: ");
    Serial.println(c);
    return 0;
  }
}

inline static uint8_t ParseHexByte(const char **in) {
  uint8_t hi = ParseHexNibble(in);
  uint8_t lo = ParseHexNibble(in);
  return hi << 4 | lo;
}

inline static void ParseAddress(const char *address, DeviceAddress *target) {
  const char *current = address;
  for (uint8_t i = 0; i < 8; i++) {
    (*target)[i] = ParseHexByte(&current);
  }
}

Thermometer::Address::Address(const String &address) {
  ParseAddress(address.c_str(), &address_);
}

Thermometer::Thermometer(const Address &address, Range validRange,
                         float calibrationOffset, String label)
    : address_(address),
      validRange_(validRange),
      label_(std::move(label)),
      calibrationOffset_(calibrationOffset),
      connected_(false),
      requested_(false),
      // lastConnected_(millis()),
      temp_() {}

bool Thermometer::requestConversion(DallasTemperature *sensors) {
  if (requested_) return true;
  // sensors->isConnected(address());
  connected_ = requested_ = sensors->requestTemperaturesByAddress(address());
}

bool Thermometer::update(DallasTemperature *sensors) {
  requested_ = false;
  Temperature reading = TempC(sensors->getTempC(address()));
  if (isWithinValidRange(reading)) {
    last_conversion_ = roo_time::Uptime::Now();
    temp_ = reading;
    return true;
  } else {
    return false;
  }
}

void ThermometerController::setup(int resolution) {
  sensors_.begin();
  sensors_.setWaitForConversion(false);
  for (int16_t i = 0; i < thermometers_size_; ++i) {
    sensors_.setResolution(thermometers_[i].address(), resolution);
    Serial.print("Resolution of thermometer ");
    Serial.print(i);
    Serial.print(" have been set to ");
    Serial.print(sensors_.getResolution(thermometers_[i].address()), DEC);
    Serial.print(" bits.");
    Serial.println();
  }
  initialized_ = true;
}

void ThermometerController::locateThermometers() {
  // locate devices on the bus
  Serial.print("Locating thermometers...");
  sensors_.begin();
  Serial.print("Found ");
  Serial.print(sensors_.getDeviceCount(), DEC);
  Serial.println(" devices.");

  // report parasite power requirements
  Serial.print("Parasite power is: ");
  if (sensors_.isParasitePowerMode()) {
    Serial.println("ON");
  } else {
    Serial.println("OFF");
  }

  for (int i = 0; i < sensors_.getDeviceCount(); ++i) {
    DeviceAddress address;
    sensors_.getAddress(address, i);
    Serial.printf("Device %d: ", i);
    printAddress(address);
    Serial.println();
  }

  Serial.println();
}

}  // namespace roo_temperature
