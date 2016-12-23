#pragma once
#include <immintrin.h>
#include "Constant.h"
#include "IFilter.h"

namespace CrossTimeDsp::Dsp
{
	private class FirstOrderFilterDouble : IFilter<double>
	{
	private:
		__int32 channels;

		__m128d a1_2;
		__m128d b0_2;
		__m128d b1_2;
		__m128d x1_2;
		__m128d y1_2;

		double a1_n;
		double b0_n;
		double b1_n;
		double* x1_n;
		double* y1_n;

	public:
		FirstOrderFilterDouble(double b0, double b1, double a0, double a1, __int32 channels);
		~FirstOrderFilterDouble();

		virtual void Filter(double* block, __int32 offset);
		virtual void FilterReverse(double* block, __int32 offset);
	};
}