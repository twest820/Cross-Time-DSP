#pragma once
#include "Constant.h"
#include "IFilter.h"
#include "Q31.h"

namespace CrossTimeDsp::Dsp
{
	private class BiquadQ31 : IFilter<__int32>
	{
	private:
		Q31* a1;
		Q31* a2;
		Q31* b0;
		Q31* b1;
		Q31* b2;
		__int32 channels;
		__int32 fractionalBitsInCoefficients;
		__int32* x1;
		__int32* x2;
		__int32* y1;
		__int32* y2;

	public:
		BiquadQ31(double b0, double b1, double b2, double a0, double a1, double a2, __int32 channels);
		~BiquadQ31();

		virtual void Filter(__int32* block, __int32 offset);
		virtual void FilterReverse(__int32* block, __int32 offset);
	};
}

