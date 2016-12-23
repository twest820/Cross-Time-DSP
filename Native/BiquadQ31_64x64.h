#pragma once
#include "IFilter.h"
#include "Q63.h"

namespace CrossTimeDsp::Dsp
{
	private class BiquadQ31_64x64 : IFilter<__int32>
	{
	private:
		Q63* a1;
		Q63* a2;
		Q63* b0;
		Q63* b1;
		Q63* b2;
		__int32 channels;
		__int32 postShift;
		__int32* x1;
		__int32* x2;
		__int64* y1;
		__int64* y2;

	public:
		BiquadQ31_64x64(double b0, double b1, double b2, double a0, double a1, double a2, __int32 channels);
		~BiquadQ31_64x64();

		virtual void Filter(__int32* block, __int32 offset);
		virtual void FilterReverse(__int32* block, __int32 offset);
	};
}
