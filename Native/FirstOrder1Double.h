#pragma once
#include <immintrin.h>
#include "Constant.h"
#include "FirstOrderCoefficients.h"
#include "IFilter.h"

namespace CrossTimeDsp::Dsp
{
	private class FirstOrder1Double : IFilter<double>
	{
	private:
		double a1;
		double b0;
		double b1;
		double* x1;
		double* y1;
		__int32 channels;

	public:
		FirstOrder1Double(FirstOrderCoefficients coefficients, __int32 channels);
		~FirstOrder1Double();

		virtual void Filter(double* block, __int32 offset);
		virtual void FilterReverse(double* block, __int32 offset);
	};
}