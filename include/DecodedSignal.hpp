#pragma once

#include <string>

struct DecodedSignal
{
    std::string name;
    double value;
    std::string unit;
    bool valid;

    // Optional discrete/enum label (e.g. motor state, direction, boolean flags).
    // When non-empty, this is the signal's value and `value`/`unit` are ignored.
    std::string text;
};
