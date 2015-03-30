// -*- C++ -*-
#ifndef OCIWEB_SETT_BIGINT_INCLUDED
#define OCIWEB_SETT_BIGINT_INCLUDED
#include <array>

class BigInt {
  class Proxy;

public:
  static const size_t MAX_DIGITS = 32;

  BigInt();

  explicit BigInt(unsigned long long num);

  explicit BigInt(const std::string& str);

  explicit operator unsigned long long() const;

  explicit operator std::string() const;

  unsigned char operator[](size_t i) const;

  Proxy operator[](size_t i);

  BigInt& operator+=(const BigInt& rhs);

  friend bool operator==(const BigInt& lhs, const BigInt& rhs);
  friend bool operator!=(const BigInt& lhs, const BigInt& rhs);

  friend bool operator<(const BigInt& lhs, const BigInt& rhs);
  friend bool operator<=(const BigInt& lhs, const BigInt& rhs);
  friend bool operator>(const BigInt& lhs, const BigInt& rhs);
  friend bool operator>=(const BigInt& lhs, const BigInt& rhs);

private:
  std::array<unsigned char, MAX_DIGITS / 2> value_ = {};

  static size_t indexof(size_t decimalPlace);

  enum class Nibble : char { LOW, HIGH };

  class Proxy {
	Nibble which_;
	unsigned char& value_;

  public:
	Proxy(Nibble which, unsigned char& value);

	operator unsigned char() const;

	Proxy& operator=(unsigned char val);
  };
};

BigInt operator+(const BigInt& lhs, const BigInt& rhs);

BigInt operator""_bi(const char* str);

std::ostream& operator<<(std::ostream& os, const BigInt& bi);

#endif
