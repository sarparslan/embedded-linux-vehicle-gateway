#include "SignalJson.hpp"

#include <gtest/gtest.h>

TEST(SignalJsonTest, WritesWholeNumbersWithoutFraction)
{
    DecodedSignal signal {"MotorRpm", 1500.0, "rpm", true, ""};
    EXPECT_EQ(toJson(signal), R"({"signal":"MotorRpm","value":1500,"unit":"rpm"})");
}

TEST(SignalJsonTest, WritesFractionalValues)
{
    DecodedSignal signal {"BatteryVoltage", 398.5, "V", true, ""};
    EXPECT_EQ(toJson(signal), R"({"signal":"BatteryVoltage","value":398.5,"unit":"V"})");
}

TEST(SignalJsonTest, WritesTextValuesAsStrings)
{
    DecodedSignal signal {"MotorState", 0.0, "", true, "Running"};
    EXPECT_EQ(toJson(signal), R"({"signal":"MotorState","value":"Running","unit":""})");
}

TEST(SignalJsonTest, EscapesSpecialCharacters)
{
    DecodedSignal signal {"Name\"With\\Quotes", 1.0, "", true, ""};
    EXPECT_EQ(toJson(signal), R"({"signal":"Name\"With\\Quotes","value":1,"unit":""})");
}
