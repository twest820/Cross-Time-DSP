#pragma once
#include <immintrin.h>
#include "BiquadCoefficients.h"
#include "Constant.h"
#include "IFilter.h"

namespace CrossTimeDsp::Dsp
{
	class __declspec(dllexport) StereoBiquad1Double256 : public IFilter<double>
	{
	private:
		__m256d a1;
		__m256d a2;
		__m256d b0;
		__m256d b1;
		__m256d b2;
		__m256d x2;
		__m256d y2;

	public:
		StereoBiquad1Double256(BiquadCoefficients coefficients);

		virtual void Filter(double* block, __int32 offset);
		virtual void FilterReverse(double* block, __int32 offset);

		void *operator new(size_t size);
		void operator delete(void* gain);
	};
}
