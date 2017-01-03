#pragma once
#include <immintrin.h>
#include "BiquadCoefficients.h"
#include "Constant.h"
#include "FirstOrderCoefficients.h"
#include "IFilter.h"

namespace CrossTimeDsp::Dsp
{
	class __declspec(dllexport) StereoBiquad2FirstOrder2Double256 : public IFilter<double>
	{
	private:
		__m256d biquad_a1;
		__m256d biquad_a2;
		__m256d biquad_b0;
		__m256d biquad_b1;
		__m256d biquad_b2;
		__m256d biquad_x1;
		__m256d biquad_x2;
		__m256d biquad_y1;
		__m256d biquad_y2;

		__m256d firstOrder_a1;
		__m256d firstOrder_b0;
		__m256d firstOrder_b1;
		__m256d firstOrder_x1;
		__m256d firstOrder_y1;

	public:
		StereoBiquad2FirstOrder2Double256(BiquadCoefficients biquad0, FirstOrderCoefficients firstOrder0, BiquadCoefficients biquad1, FirstOrderCoefficients firstOrder1);

		virtual void Filter(double* block, __int32 offset);
		virtual void FilterReverse(double* block, __int32 offset);

		void *operator new(size_t size);
		void operator delete(void* gain);
	};
}
