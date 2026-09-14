#include "calc.h"

#include <gtest/gtest.h>

TEST(CalculatorTest, AddsNumbers) {
  EXPECT_EQ(add(2, 3), 5);
}

TEST(CalculatorTest, MultipliesNumbers) {
  EXPECT_EQ(multiply(4, 3), 12);
}