#include "stdafx.h"
#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>
#include <string>
#include "Q63.h"

namespace CrossTimeDsp::Dsp
{
	Q63::Q63(double value)
		: Q63(value, Q63::GetOptimalNumberOfFractionalBits(value))
	{
	}

	Q63::Q63(double value, __int32 fractionalBits)
	{
		this->fractionalBits = fractionalBits;
		this->value = (__int64)(std::pow(2.0, (double)fractionalBits) * value);
	}

	__int32 Q63::FractionalBits()
	{
		return this->fractionalBits;
	}

	__int32 Q63::GetOptimalNumberOfFractionalBits(double value)
	{
		__int32 integerBits = std::max<__int32>(0, (__int32)std::ceil(std::log2(std::abs(value)) + 10 * std::numeric_limits<double>::epsilon()));
		if (integerBits > Q63::MaximumFractionalBits)
		{
			throw new std::invalid_argument("value " + std::to_string(value) + " is too large to fit in a 32 bit signed integer.");
		}
		return Q63::MaximumFractionalBits - integerBits;
	}

	__int32 Q63::GetOptimalNumberOfFractionalBits(std::initializer_list<double> values)
	{
		__int32 largestCommonNumberOfFractionalBits = Q63::MaximumFractionalBits;
		for each(double value in values)
		{
			largestCommonNumberOfFractionalBits = std::min(Q63::GetOptimalNumberOfFractionalBits(value), largestCommonNumberOfFractionalBits);
		}
		return largestCommonNumberOfFractionalBits;
	}

	__int32 Q63::GetQ31_64x64_PostShift(std::initializer_list<double> values)
	{
		__int32 fractionalBitsInCoefficients = Q63::GetOptimalNumberOfFractionalBits(values);
		__int32 postShift = Q63::MaximumFractionalBits - fractionalBitsInCoefficients;
		return postShift;
	}

	/// <summary>
	/// Performs a 64x32 bit multiply with 64 bit accumulation, ignoring the lowest 32 bits of the multiply.  Post shifting is the 
	/// caller's responsibility.
	/// </summary>
	__int64 Q63::operator *(__int32 y)
	{
		////   all 64 bits of the upper multiply + the upper 32 bits of the lower multiply = Q95 intermediate result truncated to Q63
		return (this->value >> 32) * (__int64)y + ((this->value & 0x00000000ffffffff) * (__int64)y >> 32);
	}

	/// <summary>
	/// Performs a 64x64 bit multiply with 64 bit accumulation, ignoring the lowest 64 bits of the multiply.  Post shifting is the 
	/// caller's responsibility.
	/// </summary>
	__int64 Q63::operator *(__int64 y)
	{
		////   all 64 bits of the upper multiply + the upper 32 bits of the lower multiplies = Q127 intermediate result truncated to Q63
		return (this->value >> 32) * (y >> 32) + ((this->value & 0x00000000ffffffff) * (y >> 32) >> 32) + ((this->value >> 32) * (y & 0x00000000FFFFFFFF) >> 32);
	}
}