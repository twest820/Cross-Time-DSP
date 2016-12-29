#pragma once
#include <initializer_list>

namespace CrossTimeDsp::Dsp
{
	private class Q31
	{
	private:
		__int32 fractionalBits;
		__int32 value;

	public:
		static const __int32 MaximumFractionalBits = 31;

		Q31(double value);
		Q31(double value, __int32 fractionalBits);

		__int32 FractionalBits();
		__int32 Value();

		static __int32 GetOptimalNumberOfFractionalBits(double value);
		static __int32 GetOptimalNumberOfFractionalBits(std::initializer_list<double> values);
		static __int32 GetQ31_32x64_PostShift(std::initializer_list<double> values);

		__int64 operator *(__int32 y);
		__int64 operator *(__int64 y);
	};
}