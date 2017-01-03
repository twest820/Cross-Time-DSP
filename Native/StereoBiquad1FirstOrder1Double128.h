#pragma once
#include <immintrin.h>
#include "BiquadCoefficients.h"
#include "Constant.h"
#include "FirstOrderCoefficients.h"
#include "IFilter.h"

namespace CrossTimeDsp::Dsp
{
	class __declspec(dllexport) StereoBiquad1FirstOrder1Double128 : public IFilter<double>
	{
	private:
		__m128d biquad_a1;
		__m128d biquad_a2;
		__m128d biquad_b0;
		__m128d biquad_b1;
		__m128d biquad_b2;
		__m128d biquad_x1;
		__m128d biquad_x2;
		__m128d biquad_y1;
		__m128d biquad_y2;

		__m128d firstOrder_a1;
		__m128d firstOrder_b0;
		__m128d firstOrder_b1;
		__m128d firstOrder_x1;
		__m128d firstOrder_y1;

	public:
		StereoBiquad1FirstOrder1Double128(BiquadCoefficients biquad, FirstOrderCoefficients firstOrder);

		virtual void Filter(double* block, __int32 offset);
		virtual void FilterReverse(double* block, __int32 offset);
	};
}
