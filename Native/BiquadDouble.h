#pragma once
#include <immintrin.h>
#include "Constant.h"
#include "IFilter.h"

namespace CrossTimeDsp::Dsp
{
	private class BiquadDouble : IFilter<double>
	{
	private:
		__int32 channels;

		__m128d a1_2;
		__m128d a2_2;
		__m128d b0_2;
		__m128d b1_2;
		__m128d b2_2;
		__m128d x1_2;
		__m128d x2_2;
		__m128d y1_2;
		__m128d y2_2;

		double a1_n;
		double a2_n;
		double b0_n;
		double b1_n;
		double b2_n;
		double* x1_n;
		double* x2_n;
		double* y1_n;
		double* y2_n;

	public:
		BiquadDouble(double b0, double b1, double b2, double a0, double a1, double a2, __int32 channels);
		~BiquadDouble();

		virtual void Filter(double* block, __int32 offset);
		virtual void FilterReverse(double* block, __int32 offset);
	};
}
