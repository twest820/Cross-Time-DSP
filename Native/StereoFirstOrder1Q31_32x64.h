#pragma once
#include "FirstOrderCoefficients.h"
#include "IFilter.h"
#include "Q31.h"

namespace CrossTimeDsp::Dsp
{
	private class StereoFirstOrder1Q31_32x64 : IFilter<__int32>
	{
	private:
		Q31* a1;
		Q31* b0;
		Q31* b1;
		__int32 postShift;
		__int32* x1;
		__int64* y1;

	public:
		StereoFirstOrder1Q31_32x64(FirstOrderCoefficients coefficients);
		~StereoFirstOrder1Q31_32x64();

		virtual void Filter(__int32* block, __int32 offset);
		virtual void FilterReverse(__int32* block, __int32 offset);
	};
}

