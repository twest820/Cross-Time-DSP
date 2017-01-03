#include "stdafx.h"
#include "StereoBiquad2FirstOrder2Double256.h"

namespace CrossTimeDsp::Dsp
{
	StereoBiquad2FirstOrder2Double256::StereoBiquad2FirstOrder2Double256(BiquadCoefficients biquad0, FirstOrderCoefficients firstOrder0, BiquadCoefficients biquad1, FirstOrderCoefficients firstOrder1)
	{
		this->biquad_a1 = _mm256_set_pd(biquad0.A1, biquad0.A1, biquad1.A1, biquad1.A1);
		this->biquad_a2 = _mm256_set_pd(biquad0.A2, biquad0.A2, biquad1.A2, biquad1.A2);
		this->biquad_b0 = _mm256_set_pd(biquad0.B0, biquad0.B0, biquad1.B0, biquad1.B0);
		this->biquad_b1 = _mm256_set_pd(biquad0.B1, biquad0.B1, biquad1.B1, biquad1.B1);
		this->biquad_b2 = _mm256_set_pd(biquad0.B2, biquad0.B2, biquad1.B2, biquad1.B2);
		this->biquad_x1 = _mm256_setzero_pd();
		this->biquad_x2 = _mm256_setzero_pd();
		this->biquad_y1 = _mm256_setzero_pd();
		this->biquad_y2 = _mm256_setzero_pd();

		this->firstOrder_a1 = _mm256_set_pd(firstOrder0.A1, firstOrder0.A1, firstOrder1.A1, firstOrder1.A1);
		this->firstOrder_b0 = _mm256_set_pd(firstOrder0.B0, firstOrder0.B0, firstOrder1.B0, firstOrder1.B0);
		this->firstOrder_b1 = _mm256_set_pd(firstOrder0.B1, firstOrder0.B1, firstOrder1.B1, firstOrder1.B1);
		this->firstOrder_x1 = _mm256_setzero_pd();
		this->firstOrder_y1 = _mm256_setzero_pd();
	}

	void StereoBiquad2FirstOrder2Double256::Filter(double* block, __int32 offset)
	{
		const __m256d BiquadLoopB0 = this->biquad_b0;
		const __m256d BiquadLoopB1 = this->biquad_b1;
		const __m256d BiquadLoopB2 = this->biquad_b2;
		const __m256d BiquadLoopA1 = this->biquad_a1;
		const __m256d BiquadLoopA2 = this->biquad_a2;
		__m256d biquadLoop_x1 = this->biquad_x1;
		__m256d biquadLoop_x2 = this->biquad_x2;
		__m256d biquadLoop_y1 = this->biquad_y1;
		__m256d biquadLoop_y2 = this->biquad_y2;

		const __m256d FirstOrderLoopB0 = this->firstOrder_b0;
		const __m256d FirstOrderLoopB1 = this->firstOrder_b1;
		const __m256d FirstOrderLoopA1 = this->firstOrder_a1;
		__m256d firstOrderLoop_x1 = this->firstOrder_x1;
		__m256d firstOrderLoop_y1 = this->firstOrder_y1;

		// run { left0, right0 } in block through biquad0 and firstOrder0
		__int32 sample = offset;
		__m128d values128 = _mm_load_pd(block + sample);
		__m128d accumulator128 = _mm_mul_pd(_mm256_castpd256_pd128(BiquadLoopB0), values128);
				accumulator128 = _mm_add_pd(accumulator128, _mm_mul_pd(_mm256_castpd256_pd128(BiquadLoopB1), _mm256_castpd256_pd128(biquadLoop_x1)));
				accumulator128 = _mm_add_pd(accumulator128, _mm_mul_pd(_mm256_castpd256_pd128(BiquadLoopB2), _mm256_castpd256_pd128(biquadLoop_x2)));
				accumulator128 = _mm_sub_pd(accumulator128, _mm_mul_pd(_mm256_castpd256_pd128(BiquadLoopA1), _mm256_castpd256_pd128(biquadLoop_y1)));
				accumulator128 = _mm_sub_pd(accumulator128, _mm_mul_pd(_mm256_castpd256_pd128(BiquadLoopA2), _mm256_castpd256_pd128(biquadLoop_y2)));
		biquadLoop_x2 = _mm256_insertf128_pd(biquadLoop_x2, _mm256_castpd256_pd128(biquadLoop_x1), Constant::Simd256::InsertLow);
		biquadLoop_x1 = _mm256_insertf128_pd(biquadLoop_x1, values128, Constant::Simd256::InsertLow);
		biquadLoop_y2 = _mm256_insertf128_pd(biquadLoop_y2, _mm256_castpd256_pd128(biquadLoop_y1), Constant::Simd256::InsertLow);
		biquadLoop_y1 = _mm256_insertf128_pd(biquadLoop_y1, accumulator128, Constant::Simd256::InsertLow);

		values128 = accumulator128;
		accumulator128 = _mm_mul_pd(_mm256_castpd256_pd128(FirstOrderLoopB0), values128);
		accumulator128 = _mm_add_pd(accumulator128, _mm_mul_pd(_mm256_castpd256_pd128(FirstOrderLoopB1), _mm256_castpd256_pd128(firstOrderLoop_x1)));
		accumulator128 = _mm_sub_pd(accumulator128, _mm_mul_pd(_mm256_castpd256_pd128(FirstOrderLoopA1), _mm256_castpd256_pd128(firstOrderLoop_y1)));
		firstOrderLoop_x1 = _mm256_insertf128_pd(firstOrderLoop_x1, values128, Constant::Simd256::InsertLow);
		firstOrderLoop_y1 = _mm256_insertf128_pd(firstOrderLoop_y1, accumulator128, Constant::Simd256::InsertLow);

		// run { leftn, rightn } through biquad0 and firstOrder0 in lane0 while thier n - 1 output goes through biquad1 and firstOrder1 in lane 1
		__int32 maxSampleForLoop = offset + Constant::FilterBlockSizeInDoubles - 2;
		__m256d previousValuesExchanged = _mm256_permute2f128_pd(firstOrderLoop_y1, firstOrderLoop_y1, Constant::Simd256::ExchangeLanes);
		while (sample < maxSampleForLoop)
		{
			__m256d values256 = _mm256_insertf128_pd(previousValuesExchanged, _mm_load_pd(block + sample + 2), Constant::Simd256::InsertLow);

			__m256d accumulator256 = _mm256_mul_pd(BiquadLoopB0, values256);
					accumulator256 = _mm256_add_pd(accumulator256, _mm256_mul_pd(BiquadLoopB1, biquadLoop_x1));
					accumulator256 = _mm256_add_pd(accumulator256, _mm256_mul_pd(BiquadLoopB2, biquadLoop_x2));
					accumulator256 = _mm256_sub_pd(accumulator256, _mm256_mul_pd(BiquadLoopA1, biquadLoop_y1));
					accumulator256 = _mm256_sub_pd(accumulator256, _mm256_mul_pd(BiquadLoopA2, biquadLoop_y2));
			biquadLoop_x2 = biquadLoop_x1;
			biquadLoop_x1 = values256;
			biquadLoop_y2 = biquadLoop_y1;
			biquadLoop_y1 = accumulator256;

			values256 = accumulator256;
			accumulator256 = _mm256_mul_pd(FirstOrderLoopB0, values256);
			accumulator256 = _mm256_add_pd(accumulator256, _mm256_mul_pd(FirstOrderLoopB1, firstOrderLoop_x1));
			accumulator256 = _mm256_sub_pd(accumulator256, _mm256_mul_pd(FirstOrderLoopA1, firstOrderLoop_y1));
			firstOrderLoop_x1 = values256;
			firstOrderLoop_y1 = accumulator256;

			previousValuesExchanged = _mm256_permute2f128_pd(firstOrderLoop_y1, firstOrderLoop_y1, Constant::Simd256::ExchangeLanes);
			_mm_store_pd(block + sample, _mm256_castpd256_pd128(previousValuesExchanged));
			sample += 2;
		}

		// run last stereo pair in block through through biquad1 and firstOrder1
		values128 = _mm256_castpd256_pd128(firstOrderLoop_y1);
		accumulator128 = _mm_mul_pd(_mm256_extractf128_pd(BiquadLoopB0, Constant::Simd256::ExtractUpper), values128);
		accumulator128 = _mm_add_pd(accumulator128, _mm_mul_pd(_mm256_extractf128_pd(BiquadLoopB1, Constant::Simd256::ExtractUpper), _mm256_extractf128_pd(biquadLoop_x1, Constant::Simd256::ExtractUpper)));
		accumulator128 = _mm_add_pd(accumulator128, _mm_mul_pd(_mm256_extractf128_pd(BiquadLoopB2, Constant::Simd256::ExtractUpper), _mm256_extractf128_pd(biquadLoop_x2, Constant::Simd256::ExtractUpper)));
		accumulator128 = _mm_sub_pd(accumulator128, _mm_mul_pd(_mm256_extractf128_pd(BiquadLoopA1, Constant::Simd256::ExtractUpper), _mm256_extractf128_pd(biquadLoop_y1, Constant::Simd256::ExtractUpper)));
		accumulator128 = _mm_sub_pd(accumulator128, _mm_mul_pd(_mm256_extractf128_pd(BiquadLoopA2, Constant::Simd256::ExtractUpper), _mm256_extractf128_pd(biquadLoop_y2, Constant::Simd256::ExtractUpper)));
		biquadLoop_x2 = _mm256_insertf128_pd(biquadLoop_x2, _mm256_extractf128_pd(biquadLoop_x1, Constant::Simd256::ExtractUpper), Constant::Simd256::InsertHigh);
		biquadLoop_x1 = _mm256_insertf128_pd(biquadLoop_x1, values128, Constant::Simd256::InsertHigh);
		biquadLoop_y2 = _mm256_insertf128_pd(biquadLoop_y2, _mm256_extractf128_pd(biquadLoop_y1, Constant::Simd256::ExtractUpper), Constant::Simd256::InsertHigh);
		biquadLoop_y1 = _mm256_insertf128_pd(biquadLoop_y1, accumulator128, Constant::Simd256::InsertHigh);

		values128 = accumulator128;
		accumulator128 = _mm_mul_pd(_mm256_extractf128_pd(FirstOrderLoopB0, Constant::Simd256::ExtractUpper), values128);
		accumulator128 = _mm_add_pd(accumulator128, _mm_mul_pd(_mm256_extractf128_pd(FirstOrderLoopB1, Constant::Simd256::ExtractUpper), _mm256_extractf128_pd(firstOrderLoop_x1, Constant::Simd256::ExtractUpper)));
		accumulator128 = _mm_sub_pd(accumulator128, _mm_mul_pd(_mm256_extractf128_pd(FirstOrderLoopA1, Constant::Simd256::ExtractUpper), _mm256_extractf128_pd(firstOrderLoop_y1, Constant::Simd256::ExtractUpper)));
		firstOrderLoop_x1 = _mm256_insertf128_pd(firstOrderLoop_x1, values128, Constant::Simd256::InsertHigh);
		firstOrderLoop_y1 = _mm256_insertf128_pd(firstOrderLoop_y1, accumulator128, Constant::Simd256::InsertHigh);

		_mm_store_pd(block + sample, accumulator128);

		this->biquad_x2 = biquadLoop_x2;
		this->biquad_x1 = biquadLoop_x1;
		this->biquad_y2 = biquadLoop_y2;
		this->biquad_y1 = biquadLoop_y1;
		this->firstOrder_x1 = firstOrderLoop_x1;
		this->firstOrder_y1 = firstOrderLoop_y1;
	}

	void StereoBiquad2FirstOrder2Double256::FilterReverse(double* block, __int32 offset)
	{
		// VS2015.3 achieves slightly better register utilzation without register hints
		const __m256d BiquadLoopB0 = this->biquad_b0;
		const __m256d BiquadLoopB1 = this->biquad_b1;
		const __m256d BiquadLoopB2 = this->biquad_b2;
		const __m256d BiquadLoopA1 = this->biquad_a1;
		const __m256d BiquadLoopA2 = this->biquad_a2;
		__m256d biquadLoop_x1 = this->biquad_x1;
		__m256d biquadLoop_x2 = this->biquad_x2;
		__m256d biquadLoop_y1 = this->biquad_y1;
		__m256d biquadLoop_y2 = this->biquad_y2;

		const __m256d FirstOrderLoopB0 = this->firstOrder_b0;
		const __m256d FirstOrderLoopB1 = this->firstOrder_b1;
		const __m256d FirstOrderLoopA1 = this->firstOrder_a1;
		__m256d firstOrderLoop_x1 = this->firstOrder_x1;
		__m256d firstOrderLoop_y1 = this->firstOrder_y1;

		// run last stereo pair in block through through biquad0 and firstOrder0
		__int32 maxSample = offset + Constant::FilterBlockSizeInDoubles;
		__int32 sample = maxSample - 2;
		__m128d values128 = _mm_load_pd(block + sample);
		__m128d accumulator128 = _mm_mul_pd(_mm256_castpd256_pd128(BiquadLoopB0), values128);
				accumulator128 = _mm_add_pd(accumulator128, _mm_mul_pd(_mm256_castpd256_pd128(BiquadLoopB1), _mm256_castpd256_pd128(biquadLoop_x1)));
				accumulator128 = _mm_add_pd(accumulator128, _mm_mul_pd(_mm256_castpd256_pd128(BiquadLoopB2), _mm256_castpd256_pd128(biquadLoop_x2)));
				accumulator128 = _mm_sub_pd(accumulator128, _mm_mul_pd(_mm256_castpd256_pd128(BiquadLoopA1), _mm256_castpd256_pd128(biquadLoop_y1)));
				accumulator128 = _mm_sub_pd(accumulator128, _mm_mul_pd(_mm256_castpd256_pd128(BiquadLoopA2), _mm256_castpd256_pd128(biquadLoop_y2)));
		biquadLoop_x2 = _mm256_insertf128_pd(biquadLoop_x2, _mm256_castpd256_pd128(biquadLoop_x1), Constant::Simd256::InsertLow);
		biquadLoop_x1 = _mm256_insertf128_pd(biquadLoop_x1, values128, Constant::Simd256::InsertLow);
		biquadLoop_y2 = _mm256_insertf128_pd(biquadLoop_y2, _mm256_castpd256_pd128(biquadLoop_y1), Constant::Simd256::InsertLow);
		biquadLoop_y1 = _mm256_insertf128_pd(biquadLoop_y1, accumulator128, Constant::Simd256::InsertLow);

		values128 = accumulator128;
		accumulator128 = _mm_mul_pd(_mm256_castpd256_pd128(FirstOrderLoopB0), values128);
		accumulator128 = _mm_add_pd(accumulator128, _mm_mul_pd(_mm256_castpd256_pd128(FirstOrderLoopB1), _mm256_castpd256_pd128(firstOrderLoop_x1)));
		accumulator128 = _mm_sub_pd(accumulator128, _mm_mul_pd(_mm256_castpd256_pd128(FirstOrderLoopA1), _mm256_castpd256_pd128(firstOrderLoop_y1)));
		firstOrderLoop_x1 = _mm256_insertf128_pd(firstOrderLoop_x1, values128, Constant::Simd256::InsertLow);
		firstOrderLoop_y1 = _mm256_insertf128_pd(firstOrderLoop_y1, accumulator128, Constant::Simd256::InsertLow);

		// run { leftn, rightn } through biquad0 and firstOrder0 in lane0 while thier n - 1 output goes through biquad1 and firstOrder1 in lane 1
		__int32 minSampleForLoop = offset + 2;
		__m256d previousValuesExchanged = _mm256_permute2f128_pd(firstOrderLoop_y1, firstOrderLoop_y1, Constant::Simd256::ExchangeLanes);
		while (sample >= minSampleForLoop)
		{
			__m256d values256 = _mm256_insertf128_pd(previousValuesExchanged, _mm_load_pd(block + sample - 2), Constant::Simd256::InsertLow);

			__m256d accumulator256 = _mm256_mul_pd(BiquadLoopB0, values256);
					accumulator256 = _mm256_add_pd(accumulator256, _mm256_mul_pd(BiquadLoopB1, biquadLoop_x1));
					accumulator256 = _mm256_add_pd(accumulator256, _mm256_mul_pd(BiquadLoopB2, biquadLoop_x2));
					accumulator256 = _mm256_sub_pd(accumulator256, _mm256_mul_pd(BiquadLoopA1, biquadLoop_y1));
					accumulator256 = _mm256_sub_pd(accumulator256, _mm256_mul_pd(BiquadLoopA2, biquadLoop_y2));
			biquadLoop_x2 = biquadLoop_x1;
			biquadLoop_x1 = values256;
			biquadLoop_y2 = biquadLoop_y1;
			biquadLoop_y1 = accumulator256;

			values256 = accumulator256;
			accumulator256 = _mm256_mul_pd(FirstOrderLoopB0, values256);
			accumulator256 = _mm256_add_pd(accumulator256, _mm256_mul_pd(FirstOrderLoopB1, firstOrderLoop_x1));
			accumulator256 = _mm256_sub_pd(accumulator256, _mm256_mul_pd(FirstOrderLoopA1, firstOrderLoop_y1));
			firstOrderLoop_x1 = values256;
			firstOrderLoop_y1 = accumulator256;

			previousValuesExchanged = _mm256_permute2f128_pd(firstOrderLoop_y1, firstOrderLoop_y1, Constant::Simd256::ExchangeLanes);
			_mm_store_pd(block + sample, _mm256_castpd256_pd128(previousValuesExchanged));
			sample -= 2;
		}

		// run { left0, right0 } through through biquad1 and firstOrder1
		values128 = _mm256_castpd256_pd128(firstOrderLoop_y1);
		accumulator128 = _mm_mul_pd(_mm256_extractf128_pd(BiquadLoopB0, Constant::Simd256::ExtractUpper), values128);
		accumulator128 = _mm_add_pd(accumulator128, _mm_mul_pd(_mm256_extractf128_pd(BiquadLoopB1, Constant::Simd256::ExtractUpper), _mm256_extractf128_pd(biquadLoop_x1, Constant::Simd256::ExtractUpper)));
		accumulator128 = _mm_add_pd(accumulator128, _mm_mul_pd(_mm256_extractf128_pd(BiquadLoopB2, Constant::Simd256::ExtractUpper), _mm256_extractf128_pd(biquadLoop_x2, Constant::Simd256::ExtractUpper)));
		accumulator128 = _mm_sub_pd(accumulator128, _mm_mul_pd(_mm256_extractf128_pd(BiquadLoopA1, Constant::Simd256::ExtractUpper), _mm256_extractf128_pd(biquadLoop_y1, Constant::Simd256::ExtractUpper)));
		accumulator128 = _mm_sub_pd(accumulator128, _mm_mul_pd(_mm256_extractf128_pd(BiquadLoopA2, Constant::Simd256::ExtractUpper), _mm256_extractf128_pd(biquadLoop_y2, Constant::Simd256::ExtractUpper)));
		biquadLoop_x2 = _mm256_insertf128_pd(biquadLoop_x2, _mm256_extractf128_pd(biquadLoop_x1, Constant::Simd256::ExtractUpper), Constant::Simd256::InsertHigh);
		biquadLoop_x1 = _mm256_insertf128_pd(biquadLoop_x1, values128, Constant::Simd256::InsertHigh);
		biquadLoop_y2 = _mm256_insertf128_pd(biquadLoop_y2, _mm256_extractf128_pd(biquadLoop_y1, Constant::Simd256::ExtractUpper), Constant::Simd256::InsertHigh);
		biquadLoop_y1 = _mm256_insertf128_pd(biquadLoop_y1, accumulator128, Constant::Simd256::InsertHigh);

		values128 = accumulator128;
		accumulator128 = _mm_mul_pd(_mm256_extractf128_pd(FirstOrderLoopB0, Constant::Simd256::ExtractUpper), values128);
		accumulator128 = _mm_add_pd(accumulator128, _mm_mul_pd(_mm256_extractf128_pd(FirstOrderLoopB1, Constant::Simd256::ExtractUpper), _mm256_extractf128_pd(firstOrderLoop_x1, Constant::Simd256::ExtractUpper)));
		accumulator128 = _mm_sub_pd(accumulator128, _mm_mul_pd(_mm256_extractf128_pd(FirstOrderLoopA1, Constant::Simd256::ExtractUpper), _mm256_extractf128_pd(firstOrderLoop_y1, Constant::Simd256::ExtractUpper)));
		firstOrderLoop_x1 = _mm256_insertf128_pd(firstOrderLoop_x1, values128, Constant::Simd256::InsertHigh);
		firstOrderLoop_y1 = _mm256_insertf128_pd(firstOrderLoop_y1, accumulator128, Constant::Simd256::InsertHigh);

		_mm_store_pd(block + sample, accumulator128);
		
		this->biquad_x2 = biquadLoop_x2;
		this->biquad_x1 = biquadLoop_x1;
		this->biquad_y2 = biquadLoop_y2;
		this->biquad_y1 = biquadLoop_y1;
		this->firstOrder_x1 = firstOrderLoop_x1;
		this->firstOrder_y1 = firstOrderLoop_y1;
	}


	void *StereoBiquad2FirstOrder2Double256::operator new(size_t size)
	{
		return _aligned_malloc(size, alignof(__m256d));
	}

	void StereoBiquad2FirstOrder2Double256::operator delete(void* gain)
	{
		_aligned_free(gain);
	}
}