#pragma once
#include <immintrin.h>

namespace CrossTimeDsp::Dsp
{
	private class BiquadM256D
	{
	private:
		__m128d a1;
		__m128d a2;
		__m128d b0;
		__m128d b1;
		__m128d b2;
		__int32 channels;
		__m128d x1;
		__m128d x2;
		__m128d y1;
		__m128d y2;

	public:
		BiquadM256D(double b0, double b1, double b2, double a0, double a1, double a2, __int32 channels);

		void Filter(double* sample, double* maxSample);
		void FilterReverse(double* sample, double* minSample);
	};
}