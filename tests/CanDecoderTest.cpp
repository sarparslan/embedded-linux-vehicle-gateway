#include "CanDecoder.hpp"

#include <initializer_list>
#include <stdexcept>
#include <string>
#include <vector>

#include <gtest/gtest.h>

namespace
{
    can_frame makeFrame(canid_t id, std::initializer_list<uint8_t> bytes)
    {
        can_frame frame {};
        frame.can_id = id;
        frame.can_dlc = static_cast<uint8_t>(bytes.size());

        uint8_t index = 0;
        for (uint8_t byte : bytes)
        {
            frame.data[index++] = byte;
        }

        return frame;
    }

    const DecodedSignal& find(const std::vector<DecodedSignal>& signals, const std::string& name)
    {
        for (const DecodedSignal& signal : signals)
        {
            if (signal.name == name)
            {
                return signal;
            }
        }

        throw std::runtime_error("signal not found: " + name);
    }
}

TEST(CanDecoderTest, DecodesMotorFrame)
{
    // Same frame as the README example: cansend vcan0 100#DC05780064020100
    CanDecoder decoder;
    auto signals = decoder.decode(makeFrame(0x100, {0xDC, 0x05, 0x78, 0x00, 0x64, 0x02, 0x01, 0x00}));

    ASSERT_EQ(signals.size(), 5u);
    EXPECT_DOUBLE_EQ(find(signals, "MotorRpm").value, 1500.0);
    EXPECT_EQ(find(signals, "MotorRpm").unit, "rpm");
    EXPECT_DOUBLE_EQ(find(signals, "MotorCurrent").value, 12.0);
    EXPECT_DOUBLE_EQ(find(signals, "MotorTemperature").value, 60.0);
    EXPECT_EQ(find(signals, "MotorState").text, "Running");
    EXPECT_EQ(find(signals, "MotorDirection").text, "Forward");
}

TEST(CanDecoderTest, DecodesBmsFrame)
{
    // Same frame as the README example: cansend vcan0 200#8C0F6AFF4B4E0206
    CanDecoder decoder;
    auto signals = decoder.decode(makeFrame(0x200, {0x8C, 0x0F, 0x6A, 0xFF, 0x4B, 0x4E, 0x02, 0x06}));

    ASSERT_EQ(signals.size(), 8u);
    EXPECT_DOUBLE_EQ(find(signals, "BatteryVoltage").value, 398.0);
    EXPECT_DOUBLE_EQ(find(signals, "BatteryCurrent").value, -15.0);
    EXPECT_DOUBLE_EQ(find(signals, "BatteryTemperature").value, 35.0);
    EXPECT_DOUBLE_EQ(find(signals, "BatterySOC").value, 78.0);
    EXPECT_EQ(find(signals, "BmsState").text, "Discharge");
    EXPECT_EQ(find(signals, "ChargeAllowed").text, "false");
    EXPECT_EQ(find(signals, "DischargeAllowed").text, "true");
    EXPECT_EQ(find(signals, "ContactorClosed").text, "true");
}

TEST(CanDecoderTest, MapsUnknownEnumValuesToUnknown)
{
    CanDecoder decoder;
    auto signals = decoder.decode(makeFrame(0x100, {0, 0, 0, 0, 40, 0x09, 0x09}));

    EXPECT_EQ(find(signals, "MotorState").text, "Unknown");
    EXPECT_EQ(find(signals, "MotorDirection").text, "Unknown");
    EXPECT_DOUBLE_EQ(find(signals, "MotorTemperature").value, 0.0);
}

TEST(CanDecoderTest, IgnoresUnknownId)
{
    CanDecoder decoder;
    EXPECT_TRUE(decoder.decode(makeFrame(0x300, {1, 2, 3, 4, 5, 6, 7, 8})).empty());
}

TEST(CanDecoderTest, IgnoresFramesShorterThanLayout)
{
    CanDecoder decoder;
    EXPECT_TRUE(decoder.decode(makeFrame(0x100, {0xDC, 0x05, 0x78, 0x00, 0x64, 0x02})).empty());
    EXPECT_TRUE(decoder.decode(makeFrame(0x200, {0x8C, 0x0F, 0x6A, 0xFF, 0x4B, 0x4E, 0x02})).empty());
}

TEST(CanDecoderTest, IgnoresExtendedRemoteAndErrorFrames)
{
    CanDecoder decoder;
    const std::initializer_list<uint8_t> payload = {0xDC, 0x05, 0x78, 0x00, 0x64, 0x02, 0x01, 0x00};

    EXPECT_TRUE(decoder.decode(makeFrame(0x100 | CAN_EFF_FLAG, payload)).empty());
    EXPECT_TRUE(decoder.decode(makeFrame(0x100 | CAN_RTR_FLAG, payload)).empty());
    EXPECT_TRUE(decoder.decode(makeFrame(0x100 | CAN_ERR_FLAG, payload)).empty());
}
