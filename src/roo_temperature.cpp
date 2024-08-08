#include "roo_temperature.h"

ROO_FLAG(char, roo_temperature_default_unit, 'C');

namespace roo_temperature {
namespace {
void TemperatureToString(const Temperature& t, char* out, int maxlen) {
  switch (GET_ROO_FLAG(roo_temperature_default_unit)) {
    case 'F': {
      if (t.isUnknown()) {
        snprintf(out, maxlen, "?°F");
      } else {
        snprintf(out, maxlen, "%g°F", t.degFahrenheit());
      }
      break;
    }
    case 'K': {
      if (t.isUnknown()) {
        snprintf(out, maxlen, "?°K");
      } else {
        snprintf(out, maxlen, "%g°K", t.degKelvin());
      }
      break;
    }
    default: {
      if (t.isUnknown()) {
        snprintf(out, maxlen, "?°C");
      } else {
        snprintf(out, maxlen, "%g°C", t.degCelcius());
      }
    }
  }
}
}  // namespace

#if defined(ESP32) || defined(ESP8266) || defined(__linux__)
std::string Temperature::asString() const {
  char out[16];
  TemperatureToString(*this, out, 16);
  return out;
}
#endif

#if defined(ARDUINO)
String Temperature::asArduinoString() const {
  char out[16];
  TemperatureToString(*this, out, 16);
  return out;
}
#endif

roo_logging::Stream& operator<<(roo_logging::Stream& os, const Temperature& t) {
  char out[16];
  TemperatureToString(t, out, 16);
  os << out;
  return os;
}

}  // namespace roo_temperature