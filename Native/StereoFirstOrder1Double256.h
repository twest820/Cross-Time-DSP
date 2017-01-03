#pragma once
#include <immintrin.h>
#include "Constant.h"
#include "FirstOrderCoefficients.h"
#include "IFilter.h"

namespace CrossTimeDsp::Dsp
{
	private class StereoFirstOrder1Double256 : IFilter<double>
	{
	private:
		__m256d a1;
		__m256d b0;
		__m256d b1;
		__m256d x1;
		__m256d y1;

	public:
		StereoFirstOrder1Double256(FirstOrderCoefficients coefficients);

		virtual void Filter(double* block, __int32 offset);
		virtual void FilterReverse(double* block, __int32 offset);
	};
}