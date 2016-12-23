#pragma once
#include "IFilter.h"
#include "Q31.h"

namespace CrossTimeDsp::Dsp
{
	private class FirstOrderFilterQ31 : IFilter<__int32>
	{
	private:
		Q31* a1;
		Q31* b0;
		Q31* b1;
		__int32 channels;
		__int32 fractionalBitsInCoefficients;
		__int32* x1;
		__int32* y1;

	public:
		FirstOrderFilterQ31(double b0, double b1, double a0, double a1, __int32 channels);
		~FirstOrderFilterQ31();

		virtual void Filter(__int32* block, __int32 offset);
		virtual void FilterReverse(__int32* block, __int32 offset);
	};
}

