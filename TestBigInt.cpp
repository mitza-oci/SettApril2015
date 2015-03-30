#include "BigInt.h"
#include "gtest/gtest.h"
#include <set>
using namespace std::string_literals;

TEST(Group1, ToAndFromInt)
{
  const auto start = 8765432109876543210ull;
  const BigInt b{start};
  for (auto i = 0u; i < 19; ++i)
  {
	EXPECT_EQ(i % 10, b[i]);
  }
  EXPECT_EQ(start, (unsigned long long)b);
}

TEST(Group1, PerDigitSubscript)
{
  BigInt b;
  b[0] = 1;
  b[1] = 2;
  EXPECT_EQ(1, b[0]);
  EXPECT_EQ(2, b[1]);
  b[0] = 3;
  EXPECT_EQ(3, b[0]);
  EXPECT_EQ(2, b[1]);
  b[1] = 4;
  EXPECT_EQ(3, b[0]);
  EXPECT_EQ(4, b[1]);
}

TEST(Group1, ToAndFromString)
{
  const auto start = "10987654321098765432109876543210"s;
  const BigInt b{start};
  for (auto i = 0u; i < 32; ++i)
  {
	EXPECT_EQ(i % 10, b[i]);
  }
  EXPECT_EQ(start, std::string{b});

  const BigInt zero;
  EXPECT_EQ("0"s, std::string{zero});
}

TEST(Group1, UDLiteral)
{
  const auto b = 10987654321098765432109876543210_bi;
  for (auto i = 0u; i < 32; ++i)
  {
	EXPECT_EQ(i % 10, b[i]);
  }
}

TEST(Group1, Streaming)
{
  const auto start = "10987654321098765432109876543210"s;
  const BigInt b{start};
  std::ostringstream oss;
  oss << b;
  EXPECT_EQ(oss.str(), start);
}

TEST(Group1, Addition)
{
  const auto
	b1 = 987654321_bi,
	b2 = 518670123_bi;
  //    1506324444
  EXPECT_EQ(1506324444_bi, b1 + b2);
}

TEST(Group1, Comparison)
{
  EXPECT_LT(01_bi, 10_bi);
  const std::set<BigInt> s = {98164507242435988_bi,
							  98164507242435987_bi,
							  84509843759803275139847591_bi,
							  98316498317509815129845_bi,
							  98316498317509815129845_bi,
							  982175911123749829025982476911_bi};
  BigInt largest;
  for (const auto& e: s)
  {
	EXPECT_GE(e, largest);
	largest = e;
  }
}
