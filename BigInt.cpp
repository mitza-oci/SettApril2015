#include "BigInt.h"
#include <algorithm>
#include <stdexcept>


// Proxy nested class:

BigInt::Proxy::Proxy(Nibble which, unsigned char& value)
  : which_(which)
  , value_(value)
{}

BigInt::Proxy::operator unsigned char() const
{
  return (which_ == Nibble::LOW) ? value_ & 0xf : (value_ >> 4);
}

BigInt::Proxy& BigInt::Proxy::operator=(unsigned char val)
{
  if (val > 9)
  {
	throw std::domain_error("value must be a single decimal digit");
  }

  value_ = (which_ == Nibble::LOW)
	? (value_ & 0xf0) | val
	: ((val << 4) | (value_ & 0xf));
  return *this;
}


// The BigInt class itself:

BigInt::BigInt()
{}

BigInt::BigInt(unsigned long long num)
{
  std::generate(value_.rbegin(), value_.rend(),
				[&]()
				{
				  const auto octet = num % 100;
				  num /= 100;
				  return (octet / 10) << 4 | octet % 10;
				});
}

BigInt::BigInt(const std::string& str)
{
  auto digit = 0u;
  std::for_each(str.rbegin(), str.rend(),
				[&](auto ch)
				{
				  (*this)[digit++] = ch - '0';
				});
}

BigInt::operator unsigned long long() const
{
  return std::accumulate(value_.begin(), value_.end(), 0ull,
						 [](auto result, auto val)
						 {
						   return result * 100 + (val >> 4) * 10 + (val & 0xf);
						 });
}

BigInt::operator std::string() const
{
  const auto s =
	std::accumulate(value_.begin(), value_.end(), std::string{},
					[](auto result, auto slot)
					{
					  const auto val = (slot >> 4) * 10 + (slot & 0xf);
					  if (!result.empty() || val)
					  {
						result += std::to_string(val);
					  }
					  return result;
					});
  return s.empty() ? "0" : s;
}

BigInt::Proxy BigInt::operator[](size_t i)
{
  if (i >= MAX_DIGITS)
  {
	throw std::out_of_range("BigInt::operator[]");
  }
  return Proxy{(i % 2) ? Nibble::HIGH : Nibble::LOW, value_[indexof(i)]};
}

unsigned char BigInt::operator[](size_t i) const
{
  if (i >= MAX_DIGITS)
  {
	throw std::out_of_range("BigInt::operator[]");
  }
  return (i % 2) ? value_[indexof(i)] >> 4 : (value_[indexof(i)] & 0xf);
}

BigInt& BigInt::operator+=(const BigInt& rhs)
{
  auto carry = false;
  for (size_t dig = 0; dig < MAX_DIGITS; ++dig)
  {
	const auto val = (*this)[dig] + rhs[dig] + carry;
	carry = val > 9;
	(*this)[dig] = val - (carry ? 10 : 0);
  }

  if (carry)
  {
	throw std::range_error("BigInt::operator+=");
  }
  return *this;
}

size_t BigInt::indexof(size_t decimalPlace)
{
  return MAX_DIGITS / 2 - 1 - decimalPlace / 2;
}


// Namespace-scoped functions:

bool operator==(const BigInt& lhs, const BigInt& rhs)
{
  return lhs.value_ == rhs.value_;
}

bool operator!=(const BigInt& lhs, const BigInt& rhs)
{
  return !(lhs == rhs);
}

bool operator<(const BigInt& lhs, const BigInt& rhs)
{
  return lhs.value_ < rhs.value_;
}

bool operator<=(const BigInt& lhs, const BigInt& rhs)
{
  return !(rhs < lhs);
}

bool operator>(const BigInt& lhs, const BigInt& rhs)
{
  return rhs < lhs;
}

bool operator>=(const BigInt& lhs, const BigInt& rhs)
{
  return !(lhs < rhs);
}

BigInt operator+(const BigInt& lhs, const BigInt& rhs)
{
  BigInt result{lhs};
  return result += rhs;
}

BigInt operator""_bi(const char* str)
{
  return BigInt{str};
}

std::ostream& operator<<(std::ostream& os, const BigInt& bi)
{
  return os << std::string(bi);
}
