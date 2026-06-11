#pragma once

#include "DecodedSignal.hpp"

#include <string>

// Serializes a decoded signal as a compact JSON object:
//   {"signal":"MotorRpm","value":1500,"unit":"rpm"}
// Enum/boolean signals use their text label as a string value.
std::string toJson(const DecodedSignal& signal);
