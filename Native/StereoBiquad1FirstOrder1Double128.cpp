#include "stdafx.h"
#include "StereoBiquad1FirstOrder1Double128.h"

namespace CrossTimeDsp::Dsp
{
	StereoBiquad1FirstOrder1Double128::StereoBiquad1FirstOrder1Double128(BiquadCoefficients biquad, FirstOrderCoefficients firstOrder)
	{
		this->biquad_a1 = _mm_set1_pd(biquad.A1);
		this->biquad_a2 = _mm_set1_pd(biquad.A2);
		this->biquad_b0 = _mm_set1_pd(biquad.B0);
		this->biquad_b1 = _mm_set1_pd(biquad.B1);
		this->biquad_b2 = _mm_set1_pd(biquad.B2);
		this->biquad_x1 = _mm_setzero_pd();
		this->biquad_x2 = _mm_setzero_pd();
		this->biquad_y1 = _mm_setzero_pd();
		this->biquad_y2 = _mm_setzero_pd();

		this->firstOrder_a1 = _mm_set1_pd(firstOrder.A1);
		this->firstOrder_b0 = _mm_set1_pd(firstOrder.B0);
		this->firstOrder_b1 = _mm_set1_pd(firstOrder.B1);
		this->firstOrder_x1 = _mm_setzero_pd();
		this->firstOrder_y1 = _mm_setzero_pd();
	}

	void StereoBiquad1FirstOrder1Double128::Filter(double* block, __int32 offset)
	{
		const register __m128d BiquadLoopB0 = this->biquad_b0;
		const register __m128d BiquadLoopB1 = this->biquad_b1;
		const register __m128d BiquadLoopB2 = this->biquad_b2;
		const register __m128d BiquadLoopA1 = this->biquad_a1;
		const register __m128d BiquadLoopA2 = this->biquad_a2;
		register __m128d biquadLoop_x1 = this->biquad_x1;
		register __m128d biquadLoop_x2 = this->biquad_x2;
		register __m128d biquadLoop_y1 = this->biquad_y1;
		register __m128d biquadLoop_y2 = this->biquad_y2;

		const register __m128d FirstOrderLoopB0 = this->firstOrder_b0;
		const register __m128d FirstOrderLoopB1 = this->firstOrder_b1;
		const register __m128d FirstOrderLoopA1 = this->firstOrder_a1;
		register __m128d firstOrderLoop_x1 = this->firstOrder_x1;
		register __m128d firstOrderLoop_y1 = this->firstOrder_y1;

		__int32 maxSample = offset + Constant::FilterBlockSizeInDoubles;
		for (__int32 sample = offset; sample < maxSample; sample += 2)
		{
			__m128d values = _mm_load_pd(block + sample);
			__m128d accumulator = _mm_mul_pd(BiquadLoopB0, values);
					accumulator = _mm_add_pd(accumulator, _mm_mul_pd(BiquadLoopB1, biquadLoop_x1));
					accumulator = _mm_add_pd(accumulator, _mm_mul_pd(BiquadLoopB2, biquadLoop_x2));
					accumulator = _mm_sub_pd(accumulator, _mm_mul_pd(BiquadLoopA1, biquadLoop_y1));
					accumulator = _mm_sub_pd(accumulator, _mm_mul_pd(BiquadLoopA2, biquadLoop_y2));
			biquadLoop_x2 = biquadLoop_x1;
			biquadLoop_x1 = values;
			biquadLoop_y2 = biquadLoop_y1;
			biquadLoop_y1 = accumulator;

			values = accumulator;
			accumulator = _mm_mul_pd(FirstOrderLoopB0, values);
			accumulator = _mm_add_pd(accumulator, _mm_mul_pd(FirstOrderLoopB1, firstOrderLoop_x1));
			accumulator = _mm_sub_pd(accumulator, _mm_mul_pd(FirstOrderLoopA1, firstOrderLoop_y1));
			firstOrderLoop_x1 = values;
			firstOrderLoop_y1 = accumulator;
			_mm_store_pd(block + sample, accumulator);
		}

		this->biquad_x2 = biquadLoop_x2;
		this->biquad_x1 = biquadLoop_x1;
		this->biquad_y2 = biquadLoop_y2;
		this->biquad_y1 = biquadLoop_y1;
		this->firstOrder_x1 = firstOrderLoop_x1;
		this->firstOrder_y1 = firstOrderLoop_y1;
	}

	void StereoBiquad1FirstOrder1Double128::FilterReverse(double* block, __int32 offset)
	{
		// VS2015.3 requires guidance in employing all 16 xmm registers to avoid loads and stores of filter coefficients and states
		// Data is typically moved directly as instruction operands and doesn't require registers.  Since this is SSE, the accumulator occupies xmm0
		// implicitly doesn't benefit from being marked register.
		const register __m128d BiquadLoopB0 = this->biquad_b0;
		const register __m128d BiquadLoopB1 = this->biquad_b1;
		const register __m128d BiquadLoopB2 = this->biquad_b2;
		const register __m128d BiquadLoopA1 = this->biquad_a1;
		const register __m128d BiquadLoopA2 = this->biquad_a2;
		register __m128d biquadLoop_x1 = this->biquad_x1;
		register __m128d biquadLoop_x2 = this->biquad_x2;
		register __m128d biquadLoop_y1 = this->biquad_y1;
		register __m128d biquadLoop_y2 = this->biquad_y2;

		const register __m128d FirstOrderLoopB0 = this->firstOrder_b0;
		const register __m128d FirstOrderLoopB1 = this->firstOrder_b1;
		const register __m128d FirstOrderLoopA1 = this->firstOrder_a1;
		register __m128d firstOrderLoop_x1 = this->firstOrder_x1;
		register __m128d firstOrderLoop_y1 = this->firstOrder_y1;

		__int32 maxSample = offset + Constant::FilterBlockSizeInDoubles;
		for (__int32 sample = maxSample - 2; sample >= offset; sample -= 2)
		{
			__m128d values = _mm_load_pd(block + sample);
			__m128d accumulator = _mm_mul_pd(BiquadLoopB0, values);
					accumulator = _mm_add_pd(accumulator, _mm_mul_pd(BiquadLoopB1, biquadLoop_x1));
					accumulator = _mm_add_pd(accumulator, _mm_mul_pd(BiquadLoopB2, biquadLoop_x2));
					accumulator = _mm_sub_pd(accumulator, _mm_mul_pd(BiquadLoopA1, biquadLoop_y1));
					accumulator = _mm_sub_pd(accumulator, _mm_mul_pd(BiquadLoopA2, biquadLoop_y2));
			biquadLoop_x2 = biquadLoop_x1;
			biquadLoop_x1 = values;
			biquadLoop_y2 = biquadLoop_y1;
			biquadLoop_y1 = accumulator;

			values = accumulator;
			accumulator = _mm_mul_pd(FirstOrderLoopB0, values);
			accumulator = _mm_add_pd(accumulator, _mm_mul_pd(FirstOrderLoopB1, firstOrderLoop_x1));
			accumulator = _mm_sub_pd(accumulator, _mm_mul_pd(FirstOrderLoopA1, firstOrderLoop_y1));
			firstOrderLoop_x1 = values;
			firstOrderLoop_y1 = accumulator;
			_mm_store_pd(block + sample, accumulator);
		}

		this->biquad_x2 = biquadLoop_x2;
		this->biquad_x1 = biquadLoop_x1;
		this->biquad_y2 = biquadLoop_y2;
		this->biquad_y1 = biquadLoop_y1;
		this->firstOrder_x1 = firstOrderLoop_x1;
		this->firstOrder_y1 = firstOrderLoop_y1;
	}
}