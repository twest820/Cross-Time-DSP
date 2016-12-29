#pragma once
#include <immintrin.h>
#include "BiquadCoefficients.h"
#include "Constant.h"
#include "FirstOrderCoefficients.h"
#include "IFilter.h"

namespace CrossTimeDsp::Dsp
{
	private class Biquad1FirstOrder1Double : IFilter<double>
	{
	private:
		__int32 channels;

		__m128d biquad_a1_2;
		__m128d biquad_a2_2;
		__m128d biquad_b0_2;
		__m128d biquad_b1_2;
		__m128d biquad_b2_2;
		__m128d biquad_x1_2;
		__m128d biquad_x2_2;
		__m128d biquad_y1_2;
		__m128d biquad_y2_2;

		__m128d firstOrder_a1_2;
		__m128d firstOrder_b0_2;
		__m128d firstOrder_b1_2;
		__m128d firstOrder_x1_2;
		__m128d firstOrder_y1_2;

		double biquad_a1;
		double biquad_a2;
		double biquad_b0;
		double biquad_b1;
		double biquad_b2;
		double* biquad_x1;
		double* biquad_x2;
		double* biquad_y1;
		double* biquad_y2;

		double firstOrder_a1;
		double firstOrder_b0;
		double firstOrder_b1;
		double* firstOrder_x1;
		double* firstOrder_y1;

	public:
		Biquad1FirstOrder1Double(BiquadCoefficients biquad, FirstOrderCoefficients firstOrder, __int32 channels);
		virtual ~Biquad1FirstOrder1Double();

		virtual void Filter(double* block, __int32 offset);
		virtual void FilterReverse(double* block, __int32 offset);
	};
}
