#include "stdafx.h"
#include "StereoFirstOrder1Double256.h"

namespace CrossTimeDsp::Dsp
{
#pragma unmanaged
	StereoFirstOrder1Double256::StereoFirstOrder1Double256(FirstOrderCoefficients coefficients)
	{
		this->a1 = _mm256_set1_pd(coefficients.A1);
		this->b0 = _mm256_set1_pd(coefficients.B0);
		this->b1 = _mm256_set1_pd(coefficients.B1);

		this->x1 = _mm256_setzero_pd();
		this->y1 = _mm256_setzero_pd();
	}

	void StereoFirstOrder1Double256::Filter(double* block, __int32 offset)
	{
		const register __m256d LoopB0 = this->b0;
		const register __m256d LoopB1 = this->b1;
		const register __m256d LoopA1 = this->a1;
		const register __m256d Zero = _mm256_set1_pd(0.0);
		register __m256d loop_x1 = this->x1;
		register __m256d loop_y1 = this->y1;

		__int32 maxSample = offset + Constant::FilterBlockSizeInDoubles;
		for (__int32 sample = offset; sample < maxSample; sample += 4)
		{
			__m256d values = _mm256_load_pd(block + sample);
			__m256d loop_x1 = _mm256_permute2f128_pd(loop_x2, values, Constant::Simd256::PermuteHighToLowAndLowToHigh2x128);
			__m256d loop_y1 = _mm256_permute2f128_pd(loop_y2, Zero, Constant::Simd256::PermuteHighToLowAndLowToHigh2x128);

			// accumulate to { left0, right0 }
			__m256d accumulator = _mm256_mul_pd(LoopB0, values);
					accumulator = _mm256_add_pd(accumulator, _mm256_mul_pd(LoopB1, loop_x1));
					accumulator = _mm256_sub_pd(accumulator, _mm256_mul_pd(LoopA1, loop_y1));
			// accumulate to { left1, right1 }
			loop_y1 = _mm256_permute2f128_pd(Zero, accumulator, Constant::Simd256::PermuteHighToLowAndLowToHigh2x128);
			accumulator = _mm256_sub_pd(accumulator, _mm256_mul_pd(LoopA1, loop_y1));

			loop_x2 = values;
			loop_y2 = accumulator;
			_mm256_store_pd(block + sample, accumulator);
		}

		this->x2 = loop_x2;
		this->y2 = loop_y2;
	}

	void StereoFirstOrder1Double256::FilterReverse(double* block, __int32 offset)
	{
		// loop body is not the same as in Filter(), unlike scalar and m128 filter implementations
		const register __m256d LoopB0 = this->b0;
		const register __m256d LoopB1 = this->b1;
		const register __m256d LoopA1 = this->a1;
		const register __m256d Zero = _mm256_set1_pd(0.0);
		register __m256d loop_x2 = this->x2;

		__int32 maxSample = offset + Constant::FilterBlockSizeInDoubles;
		for (__int32 sample = maxSample - 4; sample >= offset; sample -= 4)
		{
			__m256d values = _mm256_load_pd(block + sample);
			__m256d loop_x1 = _mm256_permute2f128_pd(values, loop_x2, Constant::Simd256::PermuteHighToLowAndLowToHigh2x128);
			__m256d loop_y1 = _mm256_permute2f128_pd(Zero, loop_y2, Constant::Simd256::PermuteHighToLowAndLowToHigh2x128);

			// accumulate to { left1, right1 }
			__m256d accumulator = _mm256_mul_pd(LoopB0, values);
			accumulator = _mm256_add_pd(accumulator, _mm256_mul_pd(LoopB1, loop_x1));
			accumulator = _mm256_sub_pd(accumulator, _mm256_mul_pd(LoopA1, loop_y1));
			// accumulate to { left0, right0 }
			loop_y1 = _mm256_permute2f128_pd(accumulator, Zero, Constant::Simd256::PermuteHighToLowAndLowToHigh2x128);
			accumulator = _mm256_sub_pd(accumulator, _mm256_mul_pd(LoopA1, loop_y1));

			loop_x2 = values;
			loop_y2 = accumulator;
			_mm256_store_pd(block + sample, accumulator);
		}

		this->x2 = loop_x2;
		this->y2 = loop_y2;
	}
}