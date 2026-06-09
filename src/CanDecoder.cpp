#include "CanDecoder.hpp"

#include <cstdint>

namespace
{
    constexpr canid_t kMotorFrameId = 0x100;
    constexpr canid_t kBmsFrameId = 0x200;

    constexpr uint8_t kMotorFrameLength = 7;
    constexpr uint8_t kBmsFrameLength = 8;

    // CAN payloads here are little-endian (Intel byte order).
    uint16_t readU16(const can_frame& frame, int startByte)
    {
        return static_cast<uint16_t>(frame.data[startByte]) |
               (static_cast<uint16_t>(frame.data[startByte + 1]) << 8);
    }

    int16_t readI16(const can_frame& frame, int startByte)
    {
        return static_cast<int16_t>(readU16(frame, startByte));
    }

    const char* motorStateToString(uint8_t value)
    {
        switch (value)
        {
            case 0:  return "Off";
            case 1:  return "Ready";
            case 2:  return "Running";
            case 3:  return "Fault";
            default: return "Unknown";
        }
    }

    const char* motorDirectionToString(uint8_t value)
    {
        switch (value)
        {
            case 0:  return "Neutral";
            case 1:  return "Forward";
            case 2:  return "Reverse";
            default: return "Unknown";
        }
    }

    const char* bmsStateToString(uint8_t value)
    {
        switch (value)
        {
            case 0:  return "Off";
            case 1:  return "Standby";
            case 2:  return "Discharge";
            case 3:  return "Charge";
            case 4:  return "Fault";
            default: return "Unknown";
        }
    }

    DecodedSignal makeNumeric(const std::string& name, double value, const std::string& unit)
    {
        return { name, value, unit, true, "" };
    }

    DecodedSignal makeState(const std::string& name, const std::string& text)
    {
        return { name, 0.0, "", true, text };
    }

    DecodedSignal makeBool(const std::string& name, bool value)
    {
        return { name, 0.0, "", true, value ? "true" : "false" };
    }
}

std::vector<DecodedSignal> CanDecoder::decode(const can_frame& frame)
{
    std::vector<DecodedSignal> signals;

    // Only standard 11-bit data frames are defined; ignore extended,
    // remote and error frames.
    if (frame.can_id & (CAN_EFF_FLAG | CAN_RTR_FLAG | CAN_ERR_FLAG))
    {
        return signals;
    }

    const canid_t id = frame.can_id & CAN_SFF_MASK;

    // Electric Motor
    //   data[0..1] RPM        (uint16)
    //   data[2..3] Current    (int16, 0.1 A)
    //   data[4]    Temperature(uint8, offset -40 C)
    //   data[5]    State      (0 Off, 1 Ready, 2 Running, 3 Fault)
    //   data[6]    Direction  (0 Neutral, 1 Forward, 2 Reverse)
    if (id == kMotorFrameId)
    {
        if (frame.can_dlc < kMotorFrameLength)
        {
            return signals;
        }

        signals.push_back(makeNumeric("MotorRpm", static_cast<double>(readU16(frame, 0)), "rpm"));
        signals.push_back(makeNumeric("MotorCurrent", readI16(frame, 2) / 10.0, "A"));
        signals.push_back(makeNumeric("MotorTemperature", static_cast<int>(frame.data[4]) - 40, "C"));
        signals.push_back(makeState("MotorState", motorStateToString(frame.data[5])));
        signals.push_back(makeState("MotorDirection", motorDirectionToString(frame.data[6])));
        return signals;
    }

    // BMS
    //   data[0..1] Voltage     (uint16, 0.1 V)
    //   data[2..3] Current     (int16, 0.1 A)
    //   data[4]    Temperature (uint8, offset -40 C)
    //   data[5]    SOC         (uint8, %)
    //   data[6]    State       (0 Off, 1 Standby, 2 Discharge, 3 Charge, 4 Fault)
    //   data[7] bit0 Charge allowed, bit1 Discharge allowed, bit2 Contactor closed
    if (id == kBmsFrameId)
    {
        if (frame.can_dlc < kBmsFrameLength)
        {
            return signals;
        }

        signals.push_back(makeNumeric("BatteryVoltage", readU16(frame, 0) / 10.0, "V"));
        signals.push_back(makeNumeric("BatteryCurrent", readI16(frame, 2) / 10.0, "A"));
        signals.push_back(makeNumeric("BatteryTemperature", static_cast<int>(frame.data[4]) - 40, "C"));
        signals.push_back(makeNumeric("BatterySOC", static_cast<double>(frame.data[5]), "%"));
        signals.push_back(makeState("BmsState", bmsStateToString(frame.data[6])));
        signals.push_back(makeBool("ChargeAllowed", frame.data[7] & 0x01));
        signals.push_back(makeBool("DischargeAllowed", frame.data[7] & 0x02));
        signals.push_back(makeBool("ContactorClosed", frame.data[7] & 0x04));
        return signals;
    }

    return signals;
}
