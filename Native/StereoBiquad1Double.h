#pragma once
#include <immintrin.h>
#include "BiquadCoefficients.h"
#include "Constant.h"
#include "IFilter.h"

namespace CrossTimeDsp::Dsp
{
	private class StereoBiquad1Double : IFilter<double>
	{
	private:
		__m128d a1;
		__m128d a2;
		__m128d b0;
		__m128d b1;
		__m128d b2;
		__m128d x1;
		__m128d x2;
		__m128d y1;
		__m128d y2;

	public:
		StereoBiquad1Double(BiquadCoefficients coefficients);

		virtual void Filter(double* block, __int32 offset);
		virtual void FilterReverse(double* block, __int32 offset);
	};
}
