#include <args_parser.h>
#include <gtest/gtest.h>
#include "../util.h"

struct UtilTest : public ::testing::Test {
    static std::vector<std::string> get_vector(const std::initializer_list<std::string>& strings)
    {
        std::vector<std::string> result{};
        for (const auto& str: strings)
        {
            result.push_back(str);
        }
        return result;
    }
};

TEST_F(UtilTest, StripTestWithDefaultArg) {
    // with default arguments
    EXPECT_EQ(util::strip(" Hello "), "Hello");
    EXPECT_EQ(util::strip(" Hello"), "Hello");
    EXPECT_EQ(util::strip("Hello "), "Hello");
    EXPECT_EQ(util::strip("Hello"), "Hello");
}

TEST_F(UtilTest, StripTestWithExplicitArg) {
    // with explicitly passed arguments
    EXPECT_EQ(util::strip(" Hello ", " "), "Hello");
    EXPECT_EQ(util::strip("Hello ", " "), "Hello");
    EXPECT_EQ(util::strip("0xHello ", "0x "), "Hello");
    EXPECT_EQ(util::strip("0xHello0x", "0x "), "Hello");
    EXPECT_EQ(util::strip("0xHe0xllo0x", "0x "), "He0xllo");
    EXPECT_EQ(util::strip("0x He0xllo 0x ", "0x "), "He0xllo");
    EXPECT_EQ(util::strip("0x0x0x0x0x0xHe0xllo 0x ", "0x "), "He0xllo");
    EXPECT_EQ(util::strip("0x0x00x 0000x0xHe0xllo 0xxxxx ", "0x "), "He0xllo");  // chars in different order too

    // if not asked, it will not strip ' ' (space char)
    EXPECT_EQ(util::strip("0123 Hello 3", "012"), "3 Hello 3");
    EXPECT_EQ(util::strip("0123 Hello 3", "0123"), " Hello ");
    EXPECT_EQ(util::strip("0123 He3llo 3", "0123"), " He3llo ");
}

TEST_F(UtilTest, SplitTest) {
    EXPECT_EQ(util::split("Hello! Testing split"), get_vector({"Hello!", "Testing", "split"}));
    EXPECT_EQ(util::split("  "), get_vector({}));
    EXPECT_EQ(util::split("Hello12 ", "12"), get_vector({"Hello",  " "}));

    EXPECT_EQ(util::split("12Hello12there!", "12"), get_vector({"Hello", "there!"}));
    EXPECT_EQ(util::split("12Hello 12there!", "12"), get_vector({"Hello ", "there!"}));
    EXPECT_EQ(util::split("12Hello 12 there!", "12"), get_vector({"Hello ", " there!"}));
    EXPECT_EQ(util::split("12 Hello 12 there!", "12"), get_vector({" Hello ", " there!"}));
}

TEST_F(UtilTest, SplitEdgeCaseTest) {
    EXPECT_EQ(util::split("Hello", "12"), get_vector({"Hello"}));
    EXPECT_EQ(util::split("Hello", "01234567890123456789"), get_vector({"Hello"}));
    EXPECT_EQ(util::split("Hello0123456789", "0123456789"), get_vector({"Hello"}));
    EXPECT_EQ(util::split("0123456789Hello", "0123456789"), get_vector({"Hello"}));
    EXPECT_EQ(util::split("0123456789Hello0123456789", "0123456789"), get_vector({"Hello"}));
}

TEST_F(UtilTest, BoolConversionTest) {
    EXPECT_EQ("false", util::to_string(false));
    EXPECT_EQ(false, util::from_string<bool>("false"));
    EXPECT_EQ(true, util::from_string<bool>("true"));
    EXPECT_EQ("true", util::to_string(true));

    // error scenarios
    EXPECT_EXCEPTION(util::from_string<bool>("False"), std::invalid_argument, "Invalid value [False] to parse to bool, expected values:[true / false].");
    EXPECT_EXCEPTION(util::from_string<bool>("F"), std::invalid_argument, "Invalid value [F] to parse to bool, expected values:[true / false].")
    EXPECT_EXCEPTION(util::from_string<bool>("True"), std::invalid_argument, "Invalid value [True] to parse to bool, expected values:[true / false].");
    EXPECT_EXCEPTION(util::from_string<bool>("T"), std::invalid_argument, "Invalid value [T] to parse to bool, expected values:[true / false].");
}

TEST_F(UtilTest, IntConversionTest) {
    // trivial int conversion test
    EXPECT_EQ("2", util::to_string(2));
    EXPECT_EQ(2, util::from_string<int>("2"));
    EXPECT_EQ(-20000, util::from_string<int>("-20000"));
    EXPECT_EQ("-20000", util::to_string(-20000));
    EXPECT_EQ(7, util::from_string<int>("007"));

    // error scenarios
    EXPECT_EXCEPTION(util::from_string<int>("-29k"), std::invalid_argument, "Invalid string [-29k] to convert to numeric type");
    EXPECT_EXCEPTION(util::from_string<int>("true"), std::invalid_argument, "Invalid string [true] to convert to numeric type");
    EXPECT_EXCEPTION(util::from_string<int>("000.7"), std::invalid_argument, "Invalid string [000.7] to convert to numeric type");
    EXPECT_EXCEPTION(util::from_string<int>(" 007"), std::invalid_argument, "Invalid string [ 007] to convert to numeric type");
    EXPECT_EXCEPTION(util::from_string<int>("007 "), std::invalid_argument, "Invalid string [007 ] to convert to numeric type");
}

TEST_F(UtilTest, DoubleConversionTest) {
    // trivial double conversion test
    EXPECT_EQ("2.000000", util::to_string(2.0));
    EXPECT_DOUBLE_EQ(2, util::from_string<double>("2"));
    EXPECT_DOUBLE_EQ(-20000, util::from_string<double>("-20000"));
    EXPECT_EQ("-20000", util::to_string(-20000));
    EXPECT_DOUBLE_EQ(0.7, util::from_string<double>("00.7"));

    // error scenarios
    EXPECT_EXCEPTION(util::from_string<double>("-29k"), std::invalid_argument, "Invalid string [-29k] to convert to numeric type");
    EXPECT_EXCEPTION(util::from_string<double>("true"), std::invalid_argument, "Invalid string [true] to convert to numeric type");
    EXPECT_EXCEPTION(util::from_string<double>(" 007"), std::invalid_argument, "Invalid string [ 007] to convert to numeric type");
    EXPECT_EXCEPTION(util::from_string<double>("007 "), std::invalid_argument, "Invalid string [007 ] to convert to numeric type");
}

TEST_F(UtilTest, CharConversionTest) {
    EXPECT_EQ('c', util::from_string<char>("c"));
    EXPECT_EQ('a', util::from_string<char>("a"));
    EXPECT_EQ(' ', util::from_string<char>(" "));

    // error scenarios
    EXPECT_EXCEPTION(util::from_string<char>(" a"), std::invalid_argument, "Can't convert [ a], size: [2] to char");
    EXPECT_EXCEPTION(util::from_string<char>("a "), std::invalid_argument, "Can't convert [a ], size: [2] to char");
    EXPECT_EXCEPTION(util::from_string<char>(" a "), std::invalid_argument, "Can't convert [ a ], size: [3] to char");
    EXPECT_EXCEPTION(util::from_string<char>("  "), std::invalid_argument, "Can't convert [  ], size: [2] to char");
}
