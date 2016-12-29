#pragma once
#include <immintrin.h>
#include "BiquadCoefficients.h"
#include "Constant.h"
#include "IFilter.h"

namespace CrossTimeDsp::Dsp
{
	private class Biquad1Double : IFilter<double>
	{
	private:
		double a1;
		double a2;
		double b0;
		double b1;
		double b2;
		double* x1;
		double* x2;
		double* y1;
		double* y2;
		__int32 channels;

	public:
		Biquad1Double(BiquadCoefficients coefficients, __int32 channels);
		virtual ~Biquad1Double();

		virtual void Filter(double* block, __int32 offset);
		virtual void FilterReverse(double* block, __int32 offset);
	};
}
