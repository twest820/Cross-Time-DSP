#include "stdafx.h"
#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <string>
#include "Q31.h"

namespace CrossTimeDsp::Dsp
{
	Q31::Q31(double value)
		: Q31(value, Q31::GetOptimalNumberOfFractionalBits(value))
	{
	}

	Q31::Q31(double value, __int32 fractionalBits)
	{
		this->fractionalBits = fractionalBits;
		this->value = (__int32)(std::pow(2.0, (double)fractionalBits) * value);
	}

	__int32 Q31::FractionalBits()
	{
		return this->fractionalBits;
	}

	__int32 Q31::Value()
	{
		return this->value;
	}

	__int32 Q31::GetOptimalNumberOfFractionalBits(double value)
	{
		__int32 integerBits = std::max(0, (__int32)std::ceil(std::log2(std::abs(value)) + 10 * std::numeric_limits<double>::epsilon()));
		if (integerBits > Q31::MaximumFractionalBits)
		{
			throw new std::invalid_argument("value: " + std::to_string(value) + " is too large to fit in a 32 bit signed integer.");
		}
		return Q31::MaximumFractionalBits - integerBits;
	}

	__int32 Q31::GetOptimalNumberOfFractionalBits(std::initializer_list<double> values)
	{
		__int32 largestCommonNumberOfFractionalBits = Q31::MaximumFractionalBits;
		for each(double value in values)
		{
			largestCommonNumberOfFractionalBits = std::min(Q31::GetOptimalNumberOfFractionalBits(value), largestCommonNumberOfFractionalBits);
		}
		return largestCommonNumberOfFractionalBits;
	}

	__int32 Q31::GetQ31_32x64_PostShift(std::initializer_list<double> values)
	{
		__int32 fractionalBitsInCoefficients = Q31::GetOptimalNumberOfFractionalBits(values);
		__int32 postShift = Q31::MaximumFractionalBits - fractionalBitsInCoefficients;
		return postShift;
	}

	/// <summary>
	/// Performs a 32x32 bit multiply with 64 bit accumulation.  Post shifting is the caller's responsibility.
	/// </summary>
	__int64 Q31::operator *(__int32 y)
	{
		return (__int64)this->value * (__int64)y;
	}

	/// <summary>
	/// Performs a 32x64 bit multiply with 64 bit accumulation, ignoring the lowest 32 bits of the multiply.  Post shifting is the 
	/// caller's responsibility.
	/// </summary>
	__int64 Q31::operator *(__int64 y)
	{
		////   all 64 bits of the upper multiply + the upper 32 bits of the lower multiply = Q95 intermediate result truncated to Q63
		return (__int64)(this->value) * (__int64)(y >> 32) + (((__int64)(this->value) * (y & 0x00000000FFFFFFFF)) >> 32);
	}
}