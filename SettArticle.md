# SETT April 2015: Introduction to C++2011/2014 for non-C++ programmers #

C++ has been updated with two recent ISO specifications (published in 2011 and 2014) and today's compilers support many of the new features.  This article will demonstrate some of those features in a way that's approachable to software developers who aren't familiar with any version C++ (or haven't used it for a long time).  

## Why C++ ##

C++ is a widely used high-level multi-paradigm programming language for system and application development.  C++ is used on all kinds of systems from small embedded chips to mobile devices, desktops, and in the data center.  Since C++ is supported on all platforms, it provides a way for a common core library to be shared between applications on different platforms (including iOS and Android). More background information on C++ can be found on http://www.stroustrup.com/C++.html and on isocpp.org.  

Bjarne Stroustrup, the creator of C++, summarized its overall design goals:

- C++’s evolution must be driven by real problems
- Don’t get involved in a sterile quest for perfection
- C++ must be useful now
- Every feature must have a reasonably obvious implementation
- Always provide a transition path
- C++ is a language, not a complete system
- Don’t try to force people to use a specific programming style
- Provide comprehensive support for each supported style 

See http://www.stroustrup.com/hopl-almost-final.pdf for more details on the initial design of C++.

Unlike other high-level languages, C++ works with the Operating System platform instead of trying to abstract it away with a "virtual machine" or "execution environment".  This gives developers the requisite control over all resource usage in the program (here "resources" include heap memory, files, network sockets, etc.).

The evolution of C++ in the 2011 and 2014 specs used the following guidelines:

- Make a better C++, not make it another language
- - Improve support for systems programming and library building
- Make C++ easier to teach and learn
- - Uniformity, stronger guarantees, help novices
- Keep compatibility as much as possible
- - Not 100% compatible with old C++ (some new keywords)

Existing codebases written in pre-C++-2011 can be incrementally migrated to C++2011/2014 resulting in a more readable and maintainable project. 

## Approaching C++2014 ##

This article approaches C++2014 as a new language.  Although C++ has its historical roots in C, knowledge of C is not required for learning C++.  In fact, starting with C may bias the programmer to using certain styles and idioms that will compile just fine in C++ but are not best practices.  The following sections of this article walk through a real-world example and explain as much as possible.  All of the code snippets below come from the GitHub repository for this article at https://github.com/mitza-oci/SettApril2015 and was tested with the GCC compiler version 4.9.2.  Other compilers should work as well, but were not tested before publication.

If you are new to C++, there will inevitably be some concepts or syntax that are unfamiliar.  I encourage you to keep reading and take a look at a few online resources to get your questions answered, such as Stack Overflow http://stackoverflow.com/questions/tagged/c%2b%2b or open an Issue at the GitHub site above.  Also please use the GitHub repository as a way to see the physical organization of the project (which code belongs in which file).  The three C++ files involved are BigInt.h (header file containing class definition and function declarations), BigInt.cpp (source code file containing function definitions), and TestBigInt.cpp (source code file for unit tests).

## BigInt Class Overview ##

Fundamental integer types in C++ range in size from char (1 byte) to long long (8 bytes on most platforms).  An 8-byte unsigned integer can store values up to about 2*10^19.  Some programs need to store and manipulate larger values without resorting to inexact floating point.  We'll develop a BigInt class that can store up to 32 decimal digits in 16 bytes.  Our BigInt class is unsigned, but it could easily be enhanced to keep track of the sign.  A further extension would be to also keep track of scale (where the decimal point goes) which would turn our BigInt into a BigDecimal, also known as a fixed-point integer.

Users of our BigInt class need not be concerned with the storage format (it's an implementation detail).  We'll use an array of 16 bytes.  Each byte in the array represents two decimal digits (packed BCD format).  Array index 0 stores the most significant two decimal digits, while array index 15 stores the least significant two digits.

Since each byte stores two digits, we will need to use bitwise operators to manipulate the two digits in the two halves of the byte (these are known as nibbles).  Fortunately users of our class don't need to use any bitwise operators.  The low nibble will store the least significant digit of the pair.  The low nibble is values 0-15 of the byte, though we will only use 0-9 since we are storing a decimal digit.  The high nibble of the byte will store the more significant digit of the pair.  The high nibble is the left four bits of the byte, and can be retrieved by shifting right four bits (x >> 4).

The BigInt class definition:

    // BigInt.h
    #include <array>
    
    class BigInt {
    public:
      static const size_t MAX_DIGITS = 32;
    
      BigInt();

	  // Other class members decscribed below...

	private:
	  std::array<unsigned char, MAX_DIGITS / 2> value_ = {};
	};    

A std::array object stores the values of the digits.  std::array is a class template of two parameters: the type of each array element (here, unsigned char) and a non-type parameter indicating the number of elements.  To use std::array, the number of elements must be known at compile time.  Other containers like std::vector store a run-time-determined number of elements.

Using std::array allows the BigInt class to not worry about any memory management issues.  Composing a class based on the standard library resource-managing types such as array, vector, string, set, map, shared_ptr, and unique_ptr (TODO: fix markdown) is a recommended strategy for enforcing separation of concerns and good design.  Programmers can create their own resource-managing classes and only then do they need to worry about writing copy constructors, move constructors, copy assignment operators, move assignment operators, and destructors.  Each of these has certain technical requirements that a class developer needs to be aware of in order for the type to work with standard library types and code generated by the compiler.

The one and only data member (value_) has an in-class initializer (= {}) which sets all array elements to 0 when objects of the BigInt class are constructed.

The other members of the BigInt class shown above are the static constant MAX_DIGITS and the default constructor.  Default constructors are constructors that can be called with no arguments.  If no constructors are declared in the class definition, the compiler will add a default constructor automatically.  Since we will be adding other constructors shortly, the default constructor is declared here.  The definition of the default constructor (not shown) consists of an empty function body.

## Converting to and from Integers ##

Users of the BigInt class can convert integers to BigInts and BigInts to integers.  Each of these conversions will be *explicit* (they will not be candidates for use in implicit conversions).

	  // within the class definition:

      explicit BigInt(unsigned long long num);
    
      explicit operator unsigned long long() const;
    
Converting from an integer to a BigInt uses the constructor shown above.  Callers of this constructor may provide arguments of smaller integer types and those arguments will be implicitly converted to unsigned long long.

Converting from a BigInt to an integer (in this case, an unsigned long long int) uses a conversion operator.  Like constructors, conversion operators have no return type and can be declared with the keyword *explicit*.  The name of the conversion operator is the keyword *operator* followed by the type that it converts to.  This particular conversion operator is a *const* member of the BigInt class since converting to an integer doesn't change the value of the BigInt object.

    // In BigInt.cpp
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

Constructing a BigInt object from an integer ("num") uses a standard library function called generate.  Like many other standard library functions known as "algorithms," generate takes iterators as its first two parameters.  Those two iterators define a range of elements to operate on.  Because the built-in integers provide an easy way to examine their least significant digits (via the modulo operator %), this constructor will traverse the value_ array backwards, from index 15 (least significant) to index 0 (most significant).  The reversed traversal is accomplished by using rbegin and rend methods instead of the usual begin and end.

The third parameter to generate is a lambda function.  Generate will execute this function once per element in the range, and assign the result of the lambda to the element.  The lambda is introduced by the symbols [&]() (TODO: markdown).  [&] indicates that this lambda may capture local objects by reference.  In this case, num is captured since it is mentioned within the lambda body.  () indicates that the lambda receives no arguments from its caller (which in this case is the implementation of generate).  The body of the lambda returns a packed representation of the value 0-99 that can be stored within one octet (byte).  Note that the object num from the outer scope is modified each time the lambda function is invoked.  The type of octet is deduced by the compiler (using the *auto* keyword).  This reduces the chance for type-mismatch errors.
    
    BigInt::operator unsigned long long() const
    {
      return std::accumulate(value_.begin(), value_.end(), 0ull,
    						 [](auto result, auto val)
    						 {
    						   return result * 100 + (val >> 4) * 10 + (val & 0xf);
    						 });
    }

Converting from a BigInt to an integer uses the accumulate algorithm from the standard library.  Accumulate is known as "fold" in most functional programming environments.  Like generate's, accumulate's parameter list starts with two iterators (this time, we use the forward traversal).  The next argument is the starting value, here the *ull* suffix after the literal 0 makes the type of this expression unsigned long long.  The return type of accumulate is determined by the type of this argument.  The final argument to accumulate is a lambda which captures no variables [] and takes two parameters (result and val).  Accumulate works by providing the lambda function the result so far and a new value.  The lambda returns back to accumulate the updated result.  After completing the iteration through the range, the result is returned back to accumulate's caller.

The lambda reads each byte of the underlying value_ array (from most significant to least significant) and converts it from the packed BCD format to a normal binary unsigned number.  If the value_ stored doesn't fit in an unsigned long long, the result will "wrap around" back to 0 and keep accumulating from there.   

The following snippet shows a unit test (using Google Test a.k.a. "gtest") verifying the basic functionality of converting to and from integers:

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

## Per-digit Indexing ##

Users of the BigInt class will need a way to read and write values above what unsigned long long can represent.  BigInt allows this by providing overloaded [] (array subscript) operators.  The value in the brackets represents the decimal digit being accessed (0 is least significant, 31 is most significant).

	  // within the class definition:

      unsigned char operator[](size_t i) const;
    
      Proxy operator[](size_t i);

Each of these operators will make use a helper function which is a private static member of the class:

    size_t BigInt::indexof(size_t decimalPlace)
    {
      return MAX_DIGITS / 2 - 1 - decimalPlace / 2;
    }

Indexof translates from the decimal place number (the i parameter to the operators) to the index of the value_ array where that decimal place is stored. 
    
    unsigned char BigInt::operator[](size_t i) const
    {
      if (i >= MAX_DIGITS)
      {
    	throw std::out_of_range("BigInt::operator[]");
      }
      return (i % 2) ? value_[indexof(i)] >> 4 : (value_[indexof(i)] & 0xf);
    }
    
The simpler of the two operators [] is called when the BigInt object is const.  In this case the value at digit index i is returned to the caller, and the caller can't modify it.  unsigned char can represent values 0-255 which is plenty since only 0-9 are possible.

    BigInt::Proxy BigInt::operator[](size_t i)
    {
      if (i >= MAX_DIGITS)
      {
    	throw std::out_of_range("BigInt::operator[]");
      }
      return Proxy{(i % 2) ? Nibble::HIGH : Nibble::LOW, value_[indexof(i)]};
    }

When the caller uses operator[] on a non-const BigInt object, the value at that decimal digit may need to be changed (not just read).  Often this is an assignment statement with the [] on the left of the =.  To make this work, the operator[] needs to return a Proxy object.  The Proxy class is defined as a private nested class inside BigInt:

      enum class Nibble : char { LOW, HIGH };
    
      class Proxy {
    	Nibble which_;
    	unsigned char& value_;
    
      public:
    	Proxy(Nibble which, unsigned char& value);
    
    	operator unsigned char() const;
    
    	Proxy& operator=(unsigned char val);
      };

The Proxy constructor is used above in operator[].  Each Proxy object keeps track of whether it's representing the low or high nibble of the byte.  The byte in question from the BigInt::value_ array is referred-to by the Proxy::value_ reference.  When the & symbol is used between the type and name in a declaration it indicates a reference and not a distinct object.  Note that it's used here for both the constructor's argument and the Proxy::value_ data member.  (It's also used as the return type of Proxy::operator= by convention so that assignments can be chained.)  Apart from construction, Proxy objects support conversion to unsigned char for reading and assignment from unsigned char for writing.

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

The entire Proxy class is really just an implementation detail of the non-const operator[] shown above.  With the operator[]s in place, we can test them as follows:

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

## Converting to and from Strings ##

Strings of decimal digits can be converted to and from BigInts.  Now that operator[] is available, it can be used to implement string conversions.  The following methods will be provided:

	// within the class definition:

    explicit BigInt(const std::string& str);
    
    explicit operator std::string() const;

	// outside the class definition:

	BigInt operator""_bi(const char* str);
	
	std::ostream& operator<<(std::ostream& os, const BigInt& bi);

In addition to the constructor from string and the conversion operator to string (which mirror the ones operating on integers), BigInt provides a *literal operator* which is invoked by the compiler when an integer literal has the suffix _bi.  The * between char and str indicates that str is a pointer to a character.  Though we prefer to deal with std::string objects, literal operators must use the raw (C-compatible) representation of a string as a pointer to the start of a run of contiguous characters in memory, ending with the 0-valued character.        

The final operator in this group is the standard output stream insertion operator.  Providing this operator allows users to insert BigInt objects into any output stream.

	BigInt::BigInt(const std::string& str)
	{
	  auto digit = 0u;
	  std::for_each(str.rbegin(), str.rend(),
					[&](auto ch)
					{
					  (*this)[digit++] = ch - '0';
					});
	}
	
Constructing a BigInt from a string of decimal digits involves iterating over the string in reverse (least significant first) and assigning each digit to the BigInt object which is under construction.

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

Converting to a string makes use of std::accumulate for string concatenation instead of integer addition.  Traversing the value_ array from most significant to least, digits are appended to the string at each step.  Complicating this implementation is the requirement that leading zeros are omitted from the string, but value 0 has a string representation of a single "0".
	
	BigInt operator""_bi(const char* str)
	{
	  return BigInt{str};
	}
	
	std::ostream& operator<<(std::ostream& os, const BigInt& bi)
	{
	  return os << std::string{bi};
	}

The literal operator and stream insertion operators merely adapt the existing "from" and "to" string conversions for convenient use.  Examples can be seen in the unit tests:

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

The standard library also provides a handful of user-defined literals which can be seen in use here: "foo"s is the std::string object containing "foo".  User-defined literals that don't start with _ are reserved for the standard library. 

## Addition ##

Many math operations are possible with BigInt, but this article will be limited to addition.  The same techniques and mechanisms can be applied to other operations (which can use either operators or named methods).

	// within the class definition:
	BigInt& operator+=(const BigInt& rhs);

	// outside the class definition:
	BigInt operator+(const BigInt& lhs, const BigInt& rhs);

Operator+= is the actual addition algorithm, whereas operator+ can simply delegate to += after copying its left-hand-side (lhs) operand.  The lhs operand of operator+= is the implicit *this (receiver object). 

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

Operator+= iterates over the decimal digits and performs the usual digit-by-digit addition.  The type of "carry" is bool but it's also used as an integer: false is 0 and true is 1.

	BigInt operator+(const BigInt& lhs, const BigInt& rhs)
	{
	  BigInt result{lhs};
	  return result += rhs;
	}

And in the unit tests...

	TEST(Group1, Addition)
	{
	  const auto b1 = 987654321_bi, b2 = 518670123_bi;
	  EXPECT_EQ(1506324444_bi, b1 + b2);
	}

## Comparison ##

All six logical comparison operators (==, !=, <, <=, >, >=) can be derived from just < and ==, which will be shown here.  The complete class definition includes all six of them, and the unit test requires >=.

	// within the class definition:

	friend bool operator==(const BigInt& lhs, const BigInt& rhs);
	
	friend bool operator<(const BigInt& lhs, const BigInt& rhs);

Although they are declared within the scope of the BigInt class definition, these operators are not members of the class.  The *friend* keyword means they belong to the same scope that BigInt belongs to but also have access to private members of BigInt.  We can see that in the .cpp file the operators are not prefixed with BigInt:: (just like operator+ above).

	bool operator==(const BigInt& lhs, const BigInt& rhs)
	{
	  return lhs.value_ == rhs.value_;
	}
	
	bool operator<(const BigInt& lhs, const BigInt& rhs)
	{
	  return lhs.value_ < rhs.value_;
	}

It turns out that the std::array value_ already provides the desired semantics for both == and < (which is one reason to store the most significant digits at array index 0), so these functions delegate to std::array.
	 
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

This unit test uses the range-based for loop.  The variable "e" will refer to a different element of set "s" each time through the loop.  Since the elements of std::set are stored in order, this loop is used by the test to validate the < operator's implementation.

## Conclusions ##

Looking back through the completed BigInt class, it has no named methods.  Everything it does is through an operator or constructor.  This isn't common, but it's to be expected in this case when we want BigInt to model built-in integers as much as possible.  This example shows the true extensibility of the C++ type system.  In the course of developing the BigInt class and its unit tests, we've seen the multiple paradigms that C++ supports work together naturally:

- Object-oriented: BigInt as a class
- Functional: use of lambdas in its implementation
- Generic: use of C++ standard library containers and algorithms

The code presented here is not necessarily the most efficient implementation possible -- it was developed focused on correctness and demonstration of C++2014 features.  Future developments could focus on performance tuning (based on profiling) and extending the functionality to signed and fixed-point numbers.

C++2011 and 2014 have many more new features that were not covered in this article.  The Cpp Reference wiki at http://en.cppreference.com/ shows all of those features in the language and standard library sections.  If you'd like to extend the BigInt class as an exercise in learning C++20011/2014, keep a browser tab open to Cpp Reference at all times. 