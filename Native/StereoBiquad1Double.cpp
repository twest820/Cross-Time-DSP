#include "stdafx.h"
#include "StereoBiquad1Double.h"

namespace CrossTimeDsp::Dsp
{
	#pragma unmanaged
	StereoBiquad1Double::StereoBiquad1Double(BiquadCoefficients coefficients)
	{
		this->a1 = _mm_set1_pd(coefficients.A1);
		this->a2 = _mm_set1_pd(coefficients.A2);
		this->b0 = _mm_set1_pd(coefficients.B0);
		this->b1 = _mm_set1_pd(coefficients.B1);
		this->b2 = _mm_set1_pd(coefficients.B2);

		this->x1 = _mm_setzero_pd();
		this->x2 = _mm_setzero_pd();
		this->y1 = _mm_setzero_pd();
		this->y2 = _mm_setzero_pd();
	}

	void StereoBiquad1Double::Filter(double* block, __int32 offset)
	{
		const register __m128d LoopB0 = this->b0;
		const register __m128d LoopB1 = this->b1;
		const register __m128d LoopB2 = this->b2;
		const register __m128d LoopA1 = this->a1;
		const register __m128d LoopA2 = this->a2;
		register __m128d loop_x1 = this->x1;
		register __m128d loop_x2 = this->x2;
		register __m128d loop_y1 = this->y1;
		register __m128d loop_y2 = this->y2;

		__int32 maxSample = offset + Constant::FilterBlockSizeInDoubles;
		for (__int32 sample = offset; sample < maxSample; sample += 2)
		{
			__m128d values = _mm_load_pd(block + sample);
			__m128d accumulator = _mm_mul_pd(LoopB0, values);
					accumulator = _mm_add_pd(accumulator, _mm_mul_pd(LoopB1, loop_x1));
					accumulator = _mm_add_pd(accumulator, _mm_mul_pd(LoopB2, loop_x2));
					accumulator = _mm_sub_pd(accumulator, _mm_mul_pd(LoopA1, loop_y1));
					accumulator = _mm_sub_pd(accumulator, _mm_mul_pd(LoopA2, loop_y2));
			loop_x2 = loop_x1;
			loop_x1 = values;
			loop_y2 = loop_y1;
			loop_y1 = accumulator;
			_mm_store_pd(block + sample, accumulator);
		}

		this->x2 = loop_x2;
		this->x1 = loop_x1;
		this->y2 = loop_y2;
		this->y1 = loop_y1;
	}

	void StereoBiquad1Double::FilterReverse(double* block, __int32 offset)
	{
		// see remarks in StereoThirdOrderDouble::FilterReverse()
		const register __m128d LoopB0 = this->b0;
		const register __m128d LoopB1 = this->b1;
		const register __m128d LoopB2 = this->b2;
		const register __m128d LoopA1 = this->a1;
		const register __m128d LoopA2 = this->a2;
		register __m128d loop_x1 = this->x1;
		register __m128d loop_x2 = this->x2;
		register __m128d loop_y1 = this->y1;
		register __m128d loop_y2 = this->y2;

		__int32 maxSample = offset + Constant::FilterBlockSizeInDoubles;
		for (__int32 sample = maxSample - 2; sample >= offset; sample -= 2)
		{
			__m128d values = _mm_load_pd(block + sample);
			__m128d accumulator = _mm_mul_pd(LoopB0, values);
					accumulator = _mm_add_pd(accumulator, _mm_mul_pd(LoopB1, loop_x1));
					accumulator = _mm_add_pd(accumulator, _mm_mul_pd(LoopB2, loop_x2));
					accumulator = _mm_sub_pd(accumulator, _mm_mul_pd(LoopA1, loop_y1));
					accumulator = _mm_sub_pd(accumulator, _mm_mul_pd(LoopA2, loop_y2));
			loop_x2 = loop_x1;
			loop_x1 = values;
			loop_y2 = loop_y1;
			loop_y1 = accumulator;
			_mm_store_pd(block + sample, accumulator);
		}

		this->x2 = loop_x2;
		this->x1 = loop_x1;
		this->y2 = loop_y2;
		this->y1 = loop_y1;
	}
}