#include "stdafx.h"
#include "IacaMarks.h"
#include "StereoBiquad1Double256.h"

namespace CrossTimeDsp::Dsp
{
	StereoBiquad1Double256::StereoBiquad1Double256(BiquadCoefficients coefficients)
	{
		this->a1 = _mm256_set1_pd(coefficients.A1);
		this->a2 = _mm256_set1_pd(coefficients.A2);
		this->b0 = _mm256_set1_pd(coefficients.B0);
		this->b1 = _mm256_set1_pd(coefficients.B1);
		this->b2 = _mm256_set1_pd(coefficients.B2);

		this->x2 = _mm256_setzero_pd();
		this->y2 = _mm256_setzero_pd();
	}

	void StereoBiquad1Double256::Filter(double* block, __int32 offset)
	{
		const register __m256d LoopB0 = this->b0;
		const register __m256d LoopB1 = this->b1;
		const register __m256d LoopB2 = this->b2;
		const register __m256d LoopA1 = this->a1;
		const register __m256d LoopA2 = this->a2;
		const register __m256d Zero = _mm256_set1_pd(0.0);
		register __m256d loop_x2 = this->x2;
		register __m256d loop_y2 = this->y2;

		__int32 maxSample = offset + Constant::FilterBlockSizeInDoubles;
		for (__int32 sample = offset; sample < maxSample; sample += 4)
		{
		#ifndef SIMD256_PERMUTE_DATA
			// loop body is not the same as in FilterReverse(), unlike scalar and m128 filter implementations
			// On Haswell cross lane permutes of filter state variables pipeline poorly with floating point operations, making this impelementation
			// significantly slower than StereoBiquad1Double128.  This form is a simpler implementation than the data permutation below but it is
			// a few percent slower.  These implementations examine alternatives to the pipelined samples used in StereoBiquad2FirstOrder2Double256,
			// which are also slower than AVX 128 but potentially not quite as much as the implementations here.
			__m256d values = _mm256_load_pd(block + sample);
			__m256d loop_x1 = _mm256_permute2f128_pd(loop_x2, values, Constant::Simd256::PermuteHighToLowAndLowToHigh2x128);
			__m256d loop_y1 = _mm256_permute2f128_pd(loop_y2, Zero, Constant::Simd256::PermuteHighToLowAndLowToHigh2x128);

			// accumulate to { left0, right0 } in low lane
			__m256d accumulator = _mm256_mul_pd(LoopB0, values);
					accumulator = _mm256_add_pd(accumulator, _mm256_mul_pd(LoopB1, loop_x1));
					accumulator = _mm256_add_pd(accumulator, _mm256_mul_pd(LoopB2, loop_x2));
					accumulator = _mm256_sub_pd(accumulator, _mm256_mul_pd(LoopA1, loop_y1));
					accumulator = _mm256_sub_pd(accumulator, _mm256_mul_pd(LoopA2, loop_y2));
			// accumulate to { left1, right1 } in high lane
			loop_y1 = _mm256_permute2f128_pd(Zero, accumulator, Constant::Simd256::PermuteHighToLowAndLowToHigh2x128);
			accumulator = _mm256_sub_pd(accumulator, _mm256_mul_pd(LoopA1, loop_y1));

			loop_x2 = values;
			loop_y2 = accumulator;
			_mm256_store_pd(block + sample, accumulator);
		#else
			// permute data rather than filter state variables
			// Slightly faster than cross lane permutes of state variables despite the large number of in lane permutes.  But still significantly slower than
			// AVX128 due to permute overhead.
			// convert load of { left0, right0, left1, right1 } to { left0, left1, right0, right1 } and form intermediate feedback terms
			// y1 high is zero as it needs to be filled from accumulator low
			__m256d load0011 = _mm256_load_pd(block + sample);
			__m256d load0101 = _mm256_permute_pd(_mm256_permute2f128_pd(load0011, load0011, Constant::Simd256::PermuteHighToLowAndLowToHigh2x128), 0x5);
			__m256d values = _mm256_blend_pd(load0011, load0101, 0x6);
			__m256d loop_x1 = _mm256_blend_pd(_mm256_permute_pd(loop_x2, 5), _mm256_permute_pd(values, 0x5), 0xa);
			__m256d loop_y1 = _mm256_blend_pd(_mm256_permute_pd(loop_y2, 5), Zero, 0xa);

			// accumulate { left0, right0 } in each lane's low
			__m256d accumulator0011 = _mm256_mul_pd(LoopB0, values);
					accumulator0011 = _mm256_add_pd(accumulator0011, _mm256_mul_pd(LoopB1, loop_x1));
					accumulator0011 = _mm256_add_pd(accumulator0011, _mm256_mul_pd(LoopB2, loop_x2));
					accumulator0011 = _mm256_sub_pd(accumulator0011, _mm256_mul_pd(LoopA1, loop_y1));
					accumulator0011 = _mm256_sub_pd(accumulator0011, _mm256_mul_pd(LoopA2, loop_y2));
			// update y1 and accumulate { left1, right1 } in each lane's high
			loop_y1 = _mm256_permute_pd(accumulator0011, 0x5);
			loop_y1 = _mm256_blend_pd(Zero, loop_y1, 0xa);
			accumulator0011 = _mm256_sub_pd(accumulator0011, _mm256_mul_pd(LoopA1, loop_y1));

			loop_x2 = values;
			loop_y2 = accumulator0011;

			// convert from { left0, left1, right0, right1 } to { left0, right0, left1, right1 } and store
			__m256d accumulator0101 = _mm256_permute_pd(_mm256_permute2f128_pd(accumulator0011, accumulator0011, Constant::Simd256::PermuteHighToLowAndLowToHigh2x128), 0x5);
			__m256d store = _mm256_blend_pd(accumulator0011, accumulator0101, 0x6);
			_mm256_store_pd(block + sample, store);
		#endif
		}

		this->x2 = loop_x2;
		this->y2 = loop_y2;
	}

	void StereoBiquad1Double256::FilterReverse(double* block, __int32 offset)
	{
		// see remarks in Filter()
		const register __m256d LoopB0 = this->b0;
		const register __m256d LoopB1 = this->b1;
		const register __m256d LoopB2 = this->b2;
		const register __m256d LoopA1 = this->a1;
		const register __m256d LoopA2 = this->a2;
		const register __m256d Zero = _mm256_set1_pd(0.0);
		register __m256d loop_x2 = this->x2;
		register __m256d loop_y2 = this->y2;

		__int32 maxSample = offset + Constant::FilterBlockSizeInDoubles;
		for (__int32 sample = maxSample - 4; sample >= offset; sample -= 4)
		{
			IACA_START
			__m256d values = _mm256_load_pd(block + sample);
			__m256d loop_x1 = _mm256_permute2f128_pd(values, loop_x2, Constant::Simd256::PermuteHighToLowAndLowToHigh2x128);
			__m256d loop_y1 = _mm256_permute2f128_pd(Zero, loop_y2, Constant::Simd256::PermuteHighToLowAndLowToHigh2x128);

			// accumulate to { left1, right1 }
			__m256d accumulator = _mm256_mul_pd(LoopB0, values);
					accumulator = _mm256_add_pd(accumulator, _mm256_mul_pd(LoopB1, loop_x1));
					accumulator = _mm256_add_pd(accumulator, _mm256_mul_pd(LoopB2, loop_x2));
					accumulator = _mm256_sub_pd(accumulator, _mm256_mul_pd(LoopA1, loop_y1));
					accumulator = _mm256_sub_pd(accumulator, _mm256_mul_pd(LoopA2, loop_y2));
			// accumulate to { left0, right0 }
			// This bottleneck contributes to degraded performance relative to AVX 128.  Extract + mac 128 + insert is ~10% slower than permute + mac 256, 
			// though.
			loop_y1 = _mm256_permute2f128_pd(accumulator, Zero, Constant::Simd256::PermuteHighToLowAndLowToHigh2x128);
			accumulator = _mm256_sub_pd(accumulator, _mm256_mul_pd(LoopA1, loop_y1));

			loop_x2 = values;
			loop_y2 = accumulator;
			_mm256_store_pd(block + sample, accumulator);
		}
		IACA_END

		this->x2 = loop_x2;
		this->y2 = loop_y2;
	}

	void *StereoBiquad1Double256::operator new(size_t size)
	{
		return _aligned_malloc(size, alignof(__m256d));
	}

	void StereoBiquad1Double256::operator delete(void* gain)
	{
		_aligned_free(gain);
	}
}
