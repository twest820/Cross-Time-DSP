#include "stdafx.h"
#include "StereoFirstOrder1Double.h"

namespace CrossTimeDsp::Dsp
{
#pragma unmanaged
	StereoFirstOrder1Double::StereoFirstOrder1Double(FirstOrderCoefficients coefficients)
	{
		this->a1 = _mm_set1_pd(coefficients.A1);
		this->b0 = _mm_set1_pd(coefficients.B0);
		this->b1 = _mm_set1_pd(coefficients.B1);

		this->x1 = _mm_setzero_pd();
		this->y1 = _mm_setzero_pd();
	}

	void StereoFirstOrder1Double::Filter(double* block, __int32 offset)
	{
		const register __m128d LoopB0 = this->b0;
		const register __m128d LoopB1 = this->b1;
		const register __m128d LoopA1 = this->a1;
		register __m128d loop_x1 = this->x1;
		register __m128d loop_y1 = this->y1;

		__int32 maxSample = offset + Constant::FilterBlockSizeInDoubles;
		for (__int32 sample = offset; sample < maxSample; sample += 2)
		{
			__m128d values = _mm_load_pd(block + sample);
			__m128d accumulator = _mm_mul_pd(LoopB0, values);
					accumulator = _mm_add_pd(accumulator, _mm_mul_pd(LoopB1, loop_x1));
					accumulator = _mm_sub_pd(accumulator, _mm_mul_pd(LoopA1, loop_y1));
			loop_x1 = values;
			loop_y1 = accumulator;
			_mm_store_pd(block + sample, accumulator);
		}

		this->x1 = loop_x1;
		this->y1 = loop_y1;

	}

	void StereoFirstOrder1Double::FilterReverse(double* block, __int32 offset)
	{
		// see remarks in StereoThirdOrderDouble::FilterReverse()
		const register __m128d LoopB0 = this->b0;
		const register __m128d LoopB1 = this->b1;
		const register __m128d LoopA1 = this->a1;
		register __m128d loop_x1 = this->x1;
		register __m128d loop_y1 = this->y1;

		__int32 maxSample = offset + Constant::FilterBlockSizeInDoubles;
		for (__int32 sample = maxSample - 2; sample >= offset; sample -= 2)
		{
			__m128d values = _mm_load_pd(block + sample);
			__m128d accumulator = _mm_mul_pd(LoopB0, values);
					accumulator = _mm_add_pd(accumulator, _mm_mul_pd(LoopB1, loop_x1));
					accumulator = _mm_sub_pd(accumulator, _mm_mul_pd(LoopA1, loop_y1));
			loop_x1 = values;
			loop_y1 = accumulator;
			_mm_store_pd(block + sample, accumulator);
		}

		this->x1 = loop_x1;
		this->y1 = loop_y1;
	}
}