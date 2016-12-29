#pragma once
#include <immintrin.h>
#include "Constant.h"
#include "FirstOrderCoefficients.h"
#include "IFilter.h"

namespace CrossTimeDsp::Dsp
{
	private class StereoFirstOrder1Double : IFilter<double>
	{
	private:
		__m128d a1;
		__m128d b0;
		__m128d b1;
		__m128d x1;
		__m128d y1;

	public:
		StereoFirstOrder1Double(FirstOrderCoefficients coefficients);

		virtual void Filter(double* block, __int32 offset);
		virtual void FilterReverse(double* block, __int32 offset);
	};
}