#ifndef AOI_ENGINE_CORE_COMMON_BASETYPES_H
#define AOI_ENGINE_CORE_COMMON_BASETYPES_H
//****************************************************************************
// BaseTypes.h -- $Id$
//
// Purpose
//   Provides a base set of frequently-used types.
//
// Indentation
//   Four characters. No tabs!
//
// Modifications
//   2014-03-13 (MM) Added a numeric, case-insensitive string less functor.
//   2014-02-18 (MM) Added more to-string-vector methods.
//   2012-08-20 (MM) Added hex converters for three numeric types.
//   2012-07-30 (MM) Added a within-epsilon comparer for doubles.
//   2012-07-29 (MM) Added string vector builders.
//   2012-07-03 (MM) Added a uuid getter.
//   2012-05-14 (MM) Added pair types and associated vector types.
//   2012-04-23 (MM) Added to-string methods.
//   2010-09-08 (MM) Added the Uncopyable class.
//   2010-09-01 (MM) Added comments about the use of String and SL.
//   2010-08-30 (MM) Added Binary as a synonym for ByteVector.
//   2010-08-29 (MM) Got rid of EmptyString; added a macro for creating
//                   string literals that match the String's value type.
//   2010-08-10 (MM) Created.
//
// Copyright (c) 2010, 2012, 2014 Xiao Sheng Guang.  All rights reserved.
//****************************************************************************

#include <cmath>
#include <functional>
#include <string>
#include <utility>
#include <vector>

// String and SL(s) must be defined as a consistent pair and
// are expected to be either of the following. The engine is
// natively narrow-char (NC) at this time.
//
//       String        SL
//       ------------  ----
//   NC: std::string   s
//   WC: std::wstring  L##s
//
// Use String::value_type as the matching character type.

#ifdef  SL
#error  String literal macro (SL) previously defined.
#undef  SL
#endif

#define SL(s) s

namespace NFG
{
namespace AOI
{
    typedef unsigned __int8  Byte;
    typedef __int32          Int32;
    typedef __int64          Int64;
    typedef std::string      String;
    using BOOL =             Int32;

    typedef std::vector<Byte>   Binary;
    typedef std::vector<Byte>   ByteVector;
    typedef std::vector<Int32>  Int32Vector;
    typedef std::vector<Int64>  Int64Vector;
    typedef std::vector<String> StringVector;

    typedef std::pair<Int32,  Int32>  Int32Pair;
    typedef std::pair<Int64,  Int64>  Int64Pair;
    typedef std::pair<String, String> StringPair;
    typedef std::pair<Int32,  String> Int32String;
    typedef std::pair<Int64,  String> Int64String;

    typedef std::vector<Int32Pair>   Int32PairVector;
    typedef std::vector<Int64Pair>   Int64PairVector;
    typedef std::vector<StringPair>  StringPairVector;
    typedef std::vector<Int32String> Int32StringVector;
    typedef std::vector<Int64String> Int64StringVector;

    struct StringEqualNoCase: public std::binary_function<String, String, bool>
    {
        bool operator()(String const &a, String const &b) const;
    };

    struct StringLessNoCase: public std::binary_function<String, String, bool>
    {
        bool operator()(String const &a, String const &b) const;
    };

    struct StringLessNoCaseNumeric: public std::binary_function<String, String, bool>
    {
        bool operator()(String const &a, String const &b) const;
    };

    String ToString(Int32);
    String ToString(Int64);
    String ToString(double);
    String ToString(size_t);

    StringVector ToStringVector(String const &);
    StringVector ToStringVector(String const &, String const &);
    StringVector ToStringVector(String const &, String const &, String const &);
    StringVector ToStringVector(String const &, String const &, String const &, String const &);
    StringVector ToStringVector(String const &, String const &, String const &, String const &, String const &);
    StringVector ToStringVector(String const &, String const &, String const &, String const &, String const &, String const &);
    StringVector ToStringVector(String const &, String const &, String const &, String const &, String const &, String const &, String const &);
    StringVector ToStringVector(String const &, String const &, String const &, String const &, String const &, String const &, String const &, String const &);

    // Returns a string of 8 or fewer characters.
    String Int32ToHex (Int32);
    // Returns a string of 16 or fewer characters.
    String Int64ToHex (Int64);
    // Returns a string of 16 characters except when the value is zero
    // and a single zero ("0") is the preferred result.
    String DoubleToHex(double, bool zeroAs0=false);

    // Requires a string of 8 or fewer characters. Zero is returned for
    // an empty string.
    Int32  HexToInt32 (String const &);
    // Requires a string of 16 or fewer characters. Zero is returned for
    // an empty string.
    Int64  HexToInt64 (String const &);
    // Requires a string of 16 character with the exceptions that the
    // string can be "0" or empty (in which case zero is returned).
    double HexToDouble(String const &);

    // Retrieves a uuid as a xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx string.
    String GetRandomUuid();

    // Returns true if the two double values are within epsilon of each
    // other.
    inline bool IsEqualWithinEpsilon(double v1, double v2, double epsilon)
    {
        return std::abs(v1 - v2) < epsilon;
    }

    // This is taken from item 6 (pp. 39) of "Effective C++, 3rd Ed." by
    // Scott Meyers. Declaring it as a private base class does the trick.
    // If you need a virtual destructor, use "boost::noncopyable" instead.
    class Uncopyable
    {
        Uncopyable(Uncopyable const &);
        Uncopyable &operator=(Uncopyable const &);

    protected:
        Uncopyable() {}
       ~Uncopyable() {} /* Intentionally not virtual. */
    };
}
}
#endif//AOI_ENGINE_CORE_COMMON_BASETYPES_H
