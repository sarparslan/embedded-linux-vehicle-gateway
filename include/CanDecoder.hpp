#pragma once

#include "DecodedSignal.hpp"

#include <vector>

#include <linux/can.h>

class CanDecoder
{
public:
    // A single CAN frame may carry several packed signals, so decoding
    // returns a list. An empty list means the frame is unknown, too short
    // for its layout, or not a standard data frame.
    std::vector<DecodedSignal> decode(const can_frame& frame);
};
