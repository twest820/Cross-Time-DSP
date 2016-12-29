#pragma once
#include <immintrin.h>
#include "FirstOrderCoefficients.h"
#include "IFilter.h"

namespace CrossTimeDsp::Dsp
{
	private class StereoFirstOrder1Q31 : IFilter<__int32>
	{
	private:
		__m128i a1;
		__m128i b0;
		__m128i b1;
		__m128i x1;
		__m128i y1;
		__int32 fractionalBitsInCoefficients;

	public:
		StereoFirstOrder1Q31(FirstOrderCoefficients coefficients);

		virtual void Filter(__int32* block, __int32 offset);
		virtual void FilterReverse(__int32* block, __int32 offset);
	};
}

