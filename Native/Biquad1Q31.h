#pragma once
#include "BiquadCoefficients.h"
#include "Constant.h"
#include "IFilter.h"
#include "Q31.h"

namespace CrossTimeDsp::Dsp
{
	class __declspec(dllexport) Biquad1Q31 : public IFilter<__int32>
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
		Biquad1Q31(BiquadCoefficients coefficients, __int32 channels);
		virtual ~Biquad1Q31();

		virtual void Filter(__int32* block, __int32 offset);
		virtual void FilterReverse(__int32* block, __int32 offset);
	};
}

