#pragma once
#include <initializer_list>

namespace CrossTimeDsp::Dsp
{
	class Q63
	{
	private:
		__int32 fractionalBits;
		__int64 value;

	public:
		static const __int32 MaximumFractionalBits = 63;

		static __int32 GetOptimalNumberOfFractionalBits(double value);
		static __int32 GetOptimalNumberOfFractionalBits(std::initializer_list<double> values);
		static __int32 GetQ31_64x64_PostShift(std::initializer_list<double> values);

		Q63(double value);
		Q63(double value, __int32 fractionalBits);

		__int32 FractionalBits();

		__int64 operator *(__int32 y);
		__int64 operator *(__int64 y);
	};
}