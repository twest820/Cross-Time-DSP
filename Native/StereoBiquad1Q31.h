#pragma once
#include <immintrin.h>
#include "BiquadCoefficients.h"
#include "Constant.h"
#include "IFilter.h"

namespace CrossTimeDsp::Dsp
{
	class __declspec(dllexport) StereoBiquad1Q31 : public IFilter<__int32>
	{
	private:
		__m128i a1;
		__m128i a2;
		__m128i b0;
		__m128i b1;
		__m128i b2;
		__m128i x1;
		__m128i x2;
		__m128i y1;
		__m128i y2;
		__int32 fractionalBitsInCoefficients;

	public:
		StereoBiquad1Q31(BiquadCoefficients coefficients);

		virtual void Filter(__int32* block, __int32 offset);
		virtual void FilterReverse(__int32* block, __int32 offset);
	};
}

