#include "SignalJson.hpp"

#include <cmath>
#include <cstdint>

#include <nlohmann/json.hpp>

std::string toJson(const DecodedSignal& signal)
{
    // ordered_json keeps the keys in insertion order: signal, value, unit.
    nlohmann::ordered_json json;
    json["signal"] = signal.name;

    if (!signal.text.empty())
    {
        json["value"] = signal.text;
    }
    else if (std::trunc(signal.value) == signal.value && std::fabs(signal.value) < 1e15)
    {
        // Whole numbers are emitted without a trailing ".0".
        json["value"] = static_cast<std::int64_t>(signal.value);
    }
    else
    {
        json["value"] = signal.value;
    }

    json["unit"] = signal.unit;

    return json.dump();
}
