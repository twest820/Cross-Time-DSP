#include "stdafx.h"
#include <initializer_list>
#include "Constant.h"
#include "Q31.h"
#include "StereoFirstOrder1Q31.h"

namespace CrossTimeDsp::Dsp
{
	StereoFirstOrder1Q31::StereoFirstOrder1Q31(FirstOrderCoefficients coefficients)
	{
		this->fractionalBitsInCoefficients = Q31::GetOptimalNumberOfFractionalBits(std::initializer_list<double>({ coefficients.A1, coefficients.B0, coefficients.B1 }));
		this->a1 = _mm_set1_epi32(Q31(coefficients.A1, this->fractionalBitsInCoefficients).Value());
		this->b0 = _mm_set1_epi32(Q31(coefficients.B0, this->fractionalBitsInCoefficients).Value());
		this->b1 = _mm_set1_epi32(Q31(coefficients.B1, this->fractionalBitsInCoefficients).Value());

		this->x1 = _mm_setzero_si128();
		this->y1 = _mm_setzero_si128();
	}

	void StereoFirstOrder1Q31::Filter(__int32* block, __int32 offset)
	{
		// see remarks in StereoBiquad1Q31::FilterReverse()
		const register __m128i LoopB0 = this->b0;
		const register __m128i LoopB1 = this->b1;
		const register __m128i LoopA1 = this->a1;
		register __m128i loop_x1 = this->x1;
		register __m128i loop_y1 = this->y1;

		__m128i* offsetBlock = reinterpret_cast<__m128i*>(block + offset);
		__int32 maxIndex = Constant::FilterBlockSizeInInt32s / 4;
		for (__int32 index = 0; index < maxIndex; ++index)
		{
			// load four samples and deinterleave from { left0, right0, left1, right1 } to { left0, left1, right0, right1 }
			__m128i values = _mm_load_si128(offsetBlock + index);
			values = _mm_shuffle_epi32(values, Constant::Simd128::DeinterleaveInterleave4x32);

			// first pair of samples
			__m128i accumulator = _mm_mul_epi32(LoopB0, values);
					accumulator = _mm_add_epi64(accumulator, _mm_mul_epi32(LoopB1, loop_x1));
					accumulator = _mm_sub_epi64(accumulator, _mm_mul_epi32(LoopA1, loop_y1));
					accumulator = _mm_srli_epi64(accumulator, this->fractionalBitsInCoefficients);
			loop_x1 = values;
			loop_y1 = accumulator;
			__m128i result = accumulator;

			// second pair of samples
			values = _mm_shuffle_epi32(values, Constant::Simd128::PermuteHighAndLow4x32);
			accumulator = _mm_mul_epi32(LoopB0, values);
			accumulator = _mm_add_epi64(accumulator, _mm_mul_epi32(LoopB1, loop_x1));
			accumulator = _mm_sub_epi64(accumulator, _mm_mul_epi32(LoopA1, loop_y1));
			accumulator = _mm_srli_epi64(accumulator, this->fractionalBitsInCoefficients);
			loop_x1 = values;
			loop_y1 = accumulator;

			// interleave back to { left0, right0, left1, right1 } and store
			accumulator = _mm_shuffle_epi32(accumulator, Constant::Simd128::PermuteHighAndLow4x32);
			result = _mm_blend_epi16(result, accumulator, Constant::Simd128::MergeLowAndHigh4x32);
			result = _mm_shuffle_epi32(result, Constant::Simd128::DeinterleaveInterleave4x32);
			_mm_store_si128(offsetBlock + index, result);
		}

		this->x1 = loop_x1;
		this->y1 = loop_y1;
	}

	void StereoFirstOrder1Q31::FilterReverse(__int32* block, __int32 offset)
	{
		// see remarks in StereoBiquad1Q31::FilterReverse()
		const register __m128i LoopB0 = this->b0;
		const register __m128i LoopB1 = this->b1;
		const register __m128i LoopA1 = this->a1;
		register __m128i loop_x1 = this->x1;
		register __m128i loop_y1 = this->y1;

		__m128i* offsetBlock = reinterpret_cast<__m128i*>(block + offset);
		__int32 maxIndex = Constant::FilterBlockSizeInInt32s / 4;
		for (__int32 index = maxIndex - 1; index >= 0; --index)
		{
			// load four samples and deinterleave from { left0, right0, left1, right1 } to { left0, left1, right0, right1 }
			__m128i values = _mm_load_si128(offsetBlock + index);
			values = _mm_shuffle_epi32(values, Constant::Simd128::DeinterleaveInterleave4x32);

			// first pair of samples
			__m128i accumulator = _mm_mul_epi32(LoopB0, values);
					accumulator = _mm_add_epi64(accumulator, _mm_mul_epi32(LoopB1, loop_x1));
					accumulator = _mm_sub_epi64(accumulator, _mm_mul_epi32(LoopA1, loop_y1));
					accumulator = _mm_srli_epi64(accumulator, this->fractionalBitsInCoefficients);
			loop_x1 = values;
			loop_y1 = accumulator;
			__m128i result = accumulator;

			// second pair of samples
			values = _mm_shuffle_epi32(values, Constant::Simd128::PermuteHighAndLow4x32);
			accumulator = _mm_mul_epi32(LoopB0, values);
			accumulator = _mm_add_epi64(accumulator, _mm_mul_epi32(LoopB1, loop_x1));
			accumulator = _mm_sub_epi64(accumulator, _mm_mul_epi32(LoopA1, loop_y1));
			accumulator = _mm_srli_epi64(accumulator, this->fractionalBitsInCoefficients);
			loop_x1 = values;
			loop_y1 = accumulator;

			// interleave back to { left0, right0, left1, right1 } and store
			accumulator = _mm_shuffle_epi32(accumulator, Constant::Simd128::PermuteHighAndLow4x32);
			result = _mm_blend_epi16(result, accumulator, Constant::Simd128::MergeLowAndHigh4x32);
			result = _mm_shuffle_epi32(result, Constant::Simd128::DeinterleaveInterleave4x32);
			_mm_store_si128(offsetBlock + index, result);
		}

		this->x1 = loop_x1;
		this->y1 = loop_y1;
	}
}