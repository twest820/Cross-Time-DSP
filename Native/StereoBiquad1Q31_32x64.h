#pragma once
#include "BiquadCoefficients.h"
#include "IFilter.h"
#include "Q31.h"

namespace CrossTimeDsp::Dsp
{
	class __declspec(dllexport) StereoBiquad1Q31_32x64 : public IFilter<__int32>
	{
	private:
		Q31* a1;
		Q31* a2;
		Q31* b0;
		Q31* b1;
		Q31* b2;
		__int32 postShift;
		__int32* x1;
		__int32* x2;
		__int64* y1;
		__int64* y2;

	public:
		StereoBiquad1Q31_32x64(BiquadCoefficients coefficients);
		virtual ~StereoBiquad1Q31_32x64();

		virtual void Filter(__int32* block, __int32 offset);
		virtual void FilterReverse(__int32* block, __int32 offset);
	};
}

