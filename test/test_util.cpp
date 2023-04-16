#include "util.h"
#include <gtest/gtest.h>
#include <exception>
#include <vector>

struct TestUtilTest : public ::testing::Test {
};

TEST_F(TestUtilTest, SimpleTest) {
    EXPECT_EXCEPTION(throw std::runtime_error("testing"), std::runtime_error, "testing");
    EXPECT_EXCEPTION(throw std::runtime_error("testing"), std::exception, "testing");
}

TEST_F(TestUtilTest, SimpleTest2) {
    std::vector<int> nums{};
    EXPECT_EXCEPTION(nums.at(1), std::exception,
                     "vector::_M_range_check: __n (which is 1) >= this->size() (which is 0)");
}