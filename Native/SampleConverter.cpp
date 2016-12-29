#include "stdafx.h"
#include <immintrin.h>
#include "Constant.h"
#include "InstructionSet.h"
#include "SampleConverter.h"

namespace CrossTimeDsp::Dsp
{
	#pragma unmanaged
	__m128i SampleConverter::Int24Max128;
	__m256i SampleConverter::Int24Max256;
	__m128i SampleConverter::Int24Min128;
	__m256i SampleConverter::Int24Min256;
	__m128i SampleConverter::ShuffleInt24ToInt32_128;
	__m128i SampleConverter::ShuffleInt32ToInt24_128;

	SampleConverter::Initializer SampleConverter::InitializerInstance;

	SampleConverter::Initializer::Initializer()
	{
		SampleConverter::Int24Max128 = _mm_set1_epi32(Constant::Int24::MaxValue);
		SampleConverter::Int24Max256 = _mm256_set1_epi32(Constant::Int24::MaxValue);
		SampleConverter::Int24Min128 = _mm_set1_epi32(Constant::Int24::MinValue);
		SampleConverter::Int24Min256 = _mm256_set1_epi32(Constant::Int24::MinValue);
		SampleConverter::ShuffleInt24ToInt32_128 = _mm_setr_epi8( 0,  1,  2, -1,  3,  4,  5, -1,  6,  7,  8, -1,  9, 10, 11, -1);
		SampleConverter::ShuffleInt32ToInt24_128 = _mm_setr_epi8( 0,  1,  2,  4,  5,  6,  8,  9, 10, 12, 13, 14, -1, -1, -1, -1);
	}

	void SampleConverter::DoubleToQ15(double* doubles, __int16* int16s, __int32 samples)
	{
		__m128i* destination = reinterpret_cast<__m128i*>(int16s);
		for (__int32 destinationIndex = 0, sample = 0; sample < samples; ++destinationIndex, sample += 8)
		{
			// load 8 doubles and pack to 32 bit integers
			__m128d pd0 = _mm_load_pd(doubles + sample);
			__m128d pd1 = _mm_load_pd(doubles + sample + 2);
			__m128i epi32_0_low = _mm_cvtpd_epi32(pd0);
			__m128i epi32_0_high = _mm_shuffle_epi32(_mm_cvtpd_epi32(pd1), Constant::Simd128::PermuteHighAndLow2x64);
			__m128i epi32_0 = _mm_blend_epi16(epi32_0_low, epi32_0_high, Constant::Simd128::MergeLowAndHigh2x64);
			epi32_0 = _mm_srai_epi32(epi32_0, Constant::ShiftBetween16BitSamplesAndQ31);

			__m128d pd2 = _mm_load_pd(doubles + sample + 4);
			__m128d pd3 = _mm_load_pd(doubles + sample + 6);
			__m128i epi32_1_low = _mm_cvtpd_epi32(pd2);
			__m128i epi32_1_high = _mm_shuffle_epi32(_mm_cvtpd_epi32(pd3), Constant::Simd128::PermuteHighAndLow2x64);
			__m128i epi32_1 = _mm_blend_epi16(epi32_1_low, epi32_1_high, Constant::Simd128::MergeLowAndHigh2x64);
			epi32_1 = _mm_srai_epi32(epi32_1, Constant::ShiftBetween16BitSamplesAndQ31);

			// pack to 16 bit integers and store
			__m128i epi16 = _mm_packs_epi32(epi32_0, epi32_1);
			_mm_store_si128(destination + destinationIndex, epi16);
		}
	}

	void SampleConverter::DoubleToQ31(double* doubles, __int32* int32s, __int32 samples)
	{
		__m128i* destination = reinterpret_cast<__m128i*>(int32s);
		for (__int32 destinationIndex = 0, sample = 0; sample < samples; ++destinationIndex, sample += 4)
		{
			// load 4 doubles and pack to 32 bit integers
			__m128d pd0 = _mm_load_pd(doubles + sample);
			__m128d pd1 = _mm_load_pd(doubles + sample + 2);
			__m128i epi32_low = _mm_cvtpd_epi32(pd0);
			__m128i epi32_high = _mm_shuffle_epi32(_mm_cvtpd_epi32(pd1), Constant::Simd128::PermuteHighAndLow2x64);
			__m128i epi32 = _mm_blend_epi16(epi32_low, epi32_high, Constant::Simd128::MergeLowAndHigh2x64);
			_mm_store_si128(destination + destinationIndex, epi32);
		}
	}

	void SampleConverter::DoubleToQ23(double* doubles, unsigned __int8* bytes, __int32 samples)
	{
		if (InstructionSet::Avx2() && Constant::Simd256ConversionEnabled)
		{
			const __m256i ShuffleInt32ToInt24_256 = _mm256_setr_m128i(SampleConverter::ShuffleInt32ToInt24_128, SampleConverter::ShuffleInt32ToInt24_128);

			__m128i* destination = reinterpret_cast<__m128i*>(bytes);
			for (__int32 destinationIndex = 0, sample = 0; sample < samples; destinationIndex += 6, sample += 32)
			{
				// load 32 doubles and pack to 32 bit integers, 16 consecutive integers in each lane
				__m256d pd0 = _mm256_load_pd(doubles + sample);
				__m256d pd1 = _mm256_load_pd(doubles + sample + 4);
				__m256i epi32_0 = _mm256_castsi128_si256(_mm256_cvtpd_epi32(pd0));
				__m256i epi32_1 = _mm256_castsi128_si256(_mm256_cvtpd_epi32(pd1));

				__m256d pd2 = _mm256_load_pd(doubles + sample + 8);
				__m256d pd3 = _mm256_load_pd(doubles + sample + 12);
				__m256i epi32_2 = _mm256_castsi128_si256(_mm256_cvtpd_epi32(pd2));
				__m256i epi32_3 = _mm256_castsi128_si256(_mm256_cvtpd_epi32(pd3));

				__m256d pd4 = _mm256_load_pd(doubles + sample + 16);
				__m256d pd5 = _mm256_load_pd(doubles + sample + 20);
				__m128i epi32_4 = _mm256_cvtpd_epi32(pd4);
				__m128i epi32_5 = _mm256_cvtpd_epi32(pd5);

				__m256d pd6 = _mm256_load_pd(doubles + sample + 24);
				__m256d pd7 = _mm256_load_pd(doubles + sample + 28);
				__m128i epi32_6 = _mm256_cvtpd_epi32(pd6);
				__m128i epi32_7 = _mm256_cvtpd_epi32(pd7);

				__m256i epi32_04 = _mm256_insertf128_si256(epi32_0, epi32_4, Constant::Simd256::InsertHigh);
				epi32_04 = _mm256_srai_epi32(epi32_04, Constant::ShiftBetween24BitSamplesAndQ31);
				epi32_04 = _mm256_min_epi32(SampleConverter::Int24Max256, epi32_04);
				epi32_04 = _mm256_max_epi32(SampleConverter::Int24Min256, epi32_04);
				epi32_04 = _mm256_shuffle_epi8(epi32_04, ShuffleInt32ToInt24_256);

				__m256i epi32_15 = _mm256_insertf128_si256(epi32_1, epi32_5, Constant::Simd256::InsertHigh);
				epi32_15 = _mm256_srai_epi32(epi32_15, Constant::ShiftBetween24BitSamplesAndQ31);
				epi32_15 = _mm256_min_epi32(SampleConverter::Int24Max256, epi32_15);
				epi32_15 = _mm256_max_epi32(SampleConverter::Int24Min256, epi32_15);
				epi32_15 = _mm256_shuffle_epi8(epi32_15, ShuffleInt32ToInt24_256);

				__m256i epi32_26 = _mm256_insertf128_si256(epi32_2, epi32_6, Constant::Simd256::InsertHigh);
				epi32_26 = _mm256_srai_epi32(epi32_26, Constant::ShiftBetween24BitSamplesAndQ31);
				epi32_26 = _mm256_min_epi32(SampleConverter::Int24Max256, epi32_26);
				epi32_26 = _mm256_max_epi32(SampleConverter::Int24Min256, epi32_26);
				epi32_26 = _mm256_shuffle_epi8(epi32_26, ShuffleInt32ToInt24_256);

				__m256i epi32_37 = _mm256_insertf128_si256(epi32_3, epi32_7, Constant::Simd256::InsertHigh);
				epi32_37 = _mm256_srai_epi32(epi32_37, Constant::ShiftBetween24BitSamplesAndQ31);
				epi32_37 = _mm256_min_epi32(SampleConverter::Int24Max256, epi32_37);
				epi32_37 = _mm256_max_epi32(SampleConverter::Int24Min256, epi32_37);
				epi32_37 = _mm256_shuffle_epi8(epi32_37, ShuffleInt32ToInt24_256);

				// pack 32 bit integers into 24 bit samples
				__m256i epi24_03 = _mm256_or_si256(epi32_04, _mm256_slli_si256(epi32_15, 12));
				__m256i epi24_14 = _mm256_or_si256(_mm256_srli_si256(epi32_15, 4), _mm256_slli_si256(epi32_26, 8));
				__m256i epi24_25 = _mm256_or_si256(_mm256_srli_si256(epi32_26, 8), _mm256_slli_si256(epi32_37, 4));

				// store low lane followed by high lane
				_mm256_storeu2_m128i(destination + destinationIndex + 3, destination + destinationIndex, epi24_03);
				_mm256_storeu2_m128i(destination + destinationIndex + 4, destination + destinationIndex + 1, epi24_14);
				_mm256_storeu2_m128i(destination + destinationIndex + 5, destination + destinationIndex + 2, epi24_25);
			}
			_mm256_zeroupper();
		}
		else
		{
			// SSE4.1: _mm_blend_epi16 and _mm_blendv_epi8, SSSE3: _mm_shuffle_epi8, rest are SSE2)
			__m128i* destination = reinterpret_cast<__m128i*>(bytes);
			for (__int32 destinationIndex = 0, sample = 0; sample < samples; destinationIndex += 3, sample += 16)
			{
				// load 16 doubles and convert to 32 bit integers
				// _mm_blend_epi32 is AVX2 so _mm_blend_epi16 is used here
				__m128d pd0 = _mm_load_pd(doubles + sample);
				__m128d pd1 = _mm_load_pd(doubles + sample + 2);
				__m128i epi32_0_low = _mm_cvtpd_epi32(pd0);
				__m128i epi32_0_high = _mm_shuffle_epi32(_mm_cvtpd_epi32(pd1), Constant::Simd128::PermuteHighAndLow2x64);
				__m128i epi32_0 = _mm_blend_epi16(epi32_0_low, epi32_0_high, Constant::Simd128::MergeLowAndHigh2x64);
				epi32_0 = _mm_srai_epi32(epi32_0, Constant::ShiftBetween24BitSamplesAndQ31);
				epi32_0 = _mm_min_epi32(SampleConverter::Int24Max128, epi32_0);
				epi32_0 = _mm_max_epi32(SampleConverter::Int24Min128, epi32_0);

				__m128d pd2 = _mm_load_pd(doubles + sample + 4);
				__m128d pd3 = _mm_load_pd(doubles + sample + 6);
				__m128i epi32_1_low = _mm_cvtpd_epi32(pd2);
				__m128i epi32_1_high = _mm_shuffle_epi32(_mm_cvtpd_epi32(pd3), Constant::Simd128::PermuteHighAndLow2x64);
				__m128i epi32_1 = _mm_blend_epi16(epi32_1_low, epi32_1_high, Constant::Simd128::MergeLowAndHigh2x64);
				epi32_1 = _mm_srai_epi32(epi32_1, Constant::ShiftBetween24BitSamplesAndQ31);
				epi32_1 = _mm_min_epi32(SampleConverter::Int24Max128, epi32_1);
				epi32_1 = _mm_max_epi32(SampleConverter::Int24Min128, epi32_1);

				__m128d pd4 = _mm_load_pd(doubles + sample + 8);
				__m128d pd5 = _mm_load_pd(doubles + sample + 10);
				__m128i epi32_2_low = _mm_cvtpd_epi32(pd4);
				__m128i epi32_2_high = _mm_shuffle_epi32(_mm_cvtpd_epi32(pd5), Constant::Simd128::PermuteHighAndLow2x64);
				__m128i epi32_2 = _mm_blend_epi16(epi32_2_low, epi32_2_high, Constant::Simd128::MergeLowAndHigh2x64);
				epi32_2 = _mm_srai_epi32(epi32_2, Constant::ShiftBetween24BitSamplesAndQ31);
				epi32_2 = _mm_min_epi32(SampleConverter::Int24Max128, epi32_2);
				epi32_2 = _mm_max_epi32(SampleConverter::Int24Min128, epi32_2);

				__m128d pd6 = _mm_load_pd(doubles + sample + 12);
				__m128d pd7 = _mm_load_pd(doubles + sample + 14);
				__m128i epi32_3_low = _mm_cvtpd_epi32(pd6);
				__m128i epi32_3_high = _mm_shuffle_epi32(_mm_cvtpd_epi32(pd7), Constant::Simd128::PermuteHighAndLow2x64);
				__m128i epi32_3 = _mm_blend_epi16(epi32_3_low, epi32_3_high, Constant::Simd128::MergeLowAndHigh2x64);
				epi32_3 = _mm_srai_epi32(epi32_3, Constant::ShiftBetween24BitSamplesAndQ31);
				epi32_3 = _mm_min_epi32(SampleConverter::Int24Max128, epi32_3);
				epi32_3 = _mm_max_epi32(SampleConverter::Int24Min128, epi32_3);

				// pack 32 bit integers into 24 bit samples
				epi32_0 = _mm_shuffle_epi8(epi32_0, SampleConverter::ShuffleInt32ToInt24_128);
				epi32_1 = _mm_shuffle_epi8(epi32_1, SampleConverter::ShuffleInt32ToInt24_128);
				epi32_2 = _mm_shuffle_epi8(epi32_2, SampleConverter::ShuffleInt32ToInt24_128);
				epi32_3 = _mm_shuffle_epi8(epi32_3, SampleConverter::ShuffleInt32ToInt24_128);

				__m128i epi24_0 = _mm_or_si128(epi32_0, _mm_slli_si128(epi32_1, 12));
				__m128i epi24_1 = _mm_or_si128(_mm_srli_si128(epi32_1, 4), _mm_slli_si128(epi32_2, 8));
				__m128i epi24_2 = _mm_or_si128(_mm_srli_si128(epi32_2, 8), _mm_slli_si128(epi32_3, 4));

				_mm_store_si128(destination + destinationIndex, epi24_0);
				_mm_store_si128(destination + destinationIndex + 1, epi24_1);
				_mm_store_si128(destination + destinationIndex + 2, epi24_2);
			}
		}
	}

	void SampleConverter::Q15ToDouble(__int16* int16s, double* doubles, __int32 samples)
	{
		if (InstructionSet::Avx2() && Constant::Simd256ConversionEnabled)
		{
			__m256i* source = reinterpret_cast<__m256i*>(int16s);
			for (__int32 sample = 0; sample < samples; sample += 16)
			{
				__m256i epi16 = _mm256_load_si256(source + sample / 16);
				
				// unpack first eight samples
				__m256i epi32_01 = _mm256_cvtepi16_epi32(_mm256_castsi256_si128(epi16));
						epi32_01 = _mm256_slli_epi32(epi32_01, Constant::ShiftBetween16BitSamplesAndQ31);
				__m128i epi32_0 = _mm256_castsi256_si128(epi32_01);
				__m256d pd0 = _mm256_cvtepi32_pd(epi32_0);
				__m128i epi32_1 = _mm256_castsi256_si128(_mm256_permute2x128_si256(epi32_01, epi32_01, Constant::Simd256::ExchangeLanes));
				__m256d pd1 = _mm256_cvtepi32_pd(epi32_1);

				// unpack second eight samples
				__m256i epi32_23 = _mm256_cvtepi16_epi32(_mm256_castsi256_si128(_mm256_permute2x128_si256(epi16, epi16, Constant::Simd256::ExchangeLanes)));
						epi32_23 = _mm256_slli_epi32(epi32_23, Constant::ShiftBetween16BitSamplesAndQ31);
				__m128i epi32_2 = _mm256_castsi256_si128(epi32_23);
				__m256d pd2 = _mm256_cvtepi32_pd(epi32_2);
				__m128i epi32_3 = _mm256_castsi256_si128(_mm256_permute2x128_si256(epi32_23, epi32_23, Constant::Simd256::ExchangeLanes));
				__m256d pd3 = _mm256_cvtepi32_pd(epi32_3);

				_mm256_store_pd(doubles + sample, pd0);
				_mm256_store_pd(doubles + sample + 4, pd1);
				_mm256_store_pd(doubles + sample + 8, pd2);
				_mm256_store_pd(doubles + sample + 12, pd3);
			}
			_mm256_zeroupper();
		}
		else
		{
			// SSE4.1 (Intel Core, Penryn, 2008): _mm_cvtepi16_epi32, rest are SSE2
			__m128i* source = reinterpret_cast<__m128i*>(int16s);
			for (__int32 sample = 0; sample < samples; sample += 8)
			{
				__m128i epi16 = _mm_load_si128(source + sample / 8);

				__m128i epi32_0 = _mm_cvtepi16_epi32(epi16);
				epi32_0 = _mm_slli_epi32(epi32_0, Constant::ShiftBetween16BitSamplesAndQ31);
				_mm_store_pd(doubles + sample, _mm_cvtepi32_pd(epi32_0));
				_mm_store_pd(doubles + sample + 2, _mm_cvtepi32_pd(_mm_shuffle_epi32(epi32_0, Constant::Simd128::PermuteHighAndLow2x64)));

				__m128i epi32_1 = _mm_cvtepi16_epi32(_mm_shuffle_epi32(epi16, 0x4e));
				epi32_1 = _mm_slli_epi32(epi32_1, Constant::ShiftBetween16BitSamplesAndQ31);
				_mm_store_pd(doubles + sample + 4, _mm_cvtepi32_pd(epi32_1));
				_mm_store_pd(doubles + sample + 6, _mm_cvtepi32_pd(_mm_shuffle_epi32(epi32_1, Constant::Simd128::PermuteHighAndLow2x64)));
			}
		}
	}

	void SampleConverter::Q15ToQ31(__int16* int16s, __int32* int32s, __int32 samples)
	{
		// SSE2
		__m128i* source = reinterpret_cast<__m128i*>(int16s);
		__m128i* destination = reinterpret_cast<__m128i*>(int32s);
		for (__int32 destinationIndex = 0, sample = 0; sample < samples; destinationIndex += 2, sample += 8)
		{
			__m128i epi16 = _mm_load_si128(source + sample / 8);

			__m128i epi32_0 = _mm_cvtepi16_epi32(epi16);
			epi32_0 = _mm_slli_epi32(epi32_0, Constant::ShiftBetween16BitSamplesAndQ31);

			__m128i epi32_1 = _mm_cvtepi16_epi32(_mm_shuffle_epi32(epi16, 0x4e));
			epi32_1 = _mm_slli_epi32(epi32_1, Constant::ShiftBetween16BitSamplesAndQ31);

			_mm_store_si128(destination + destinationIndex, epi32_0);
			_mm_store_si128(destination + destinationIndex + 1, epi32_1);
		}
	}

	void SampleConverter::Q23ToDouble(unsigned __int8* bytes, double* doubles, __int32 samples)
	{
		// SSSE3: _mm_alignr_epi8
		__m128i* source = reinterpret_cast<__m128i*>(bytes);
		for (__int32 sample = 0, sourceIndex = 0; sample < samples; sample += 16, sourceIndex += 3)
		{
			// read 16 24 bit integers and unpack to 32 bit integers
			__m128i epi24_0 = _mm_load_si128(source + sourceIndex);
			__m128i epi24_1 = _mm_load_si128(source + sourceIndex + 1);
			__m128i epi24_2 = _mm_load_si128(source + sourceIndex + 2);

			__m128i epi32_0 = _mm_shuffle_epi8(epi24_0, SampleConverter::ShuffleInt24ToInt32_128);
			__m128i epi32_1 = _mm_shuffle_epi8(_mm_alignr_epi8(epi24_1, epi24_0, 12), SampleConverter::ShuffleInt24ToInt32_128);
			__m128i epi32_2 = _mm_shuffle_epi8(_mm_alignr_epi8(epi24_2, epi24_1, 8), SampleConverter::ShuffleInt24ToInt32_128);
			__m128i epi32_3 = _mm_shuffle_epi8(_mm_alignr_epi8(epi24_2, epi24_2, 4), SampleConverter::ShuffleInt24ToInt32_128);

			// shift from Q23 to Q31
			// See remarks in Q23ToQ31().
			epi32_0 = _mm_srai_epi32(_mm_slli_epi32(epi32_0, Constant::Int24::LeftShiftFromInt24ToInt32), Constant::Int24::RightShiftFromInt32ToQ31);
			epi32_1 = _mm_srai_epi32(_mm_slli_epi32(epi32_1, Constant::Int24::LeftShiftFromInt24ToInt32), Constant::Int24::RightShiftFromInt32ToQ31);
			epi32_2 = _mm_srai_epi32(_mm_slli_epi32(epi32_2, Constant::Int24::LeftShiftFromInt24ToInt32), Constant::Int24::RightShiftFromInt32ToQ31);
			epi32_3 = _mm_srai_epi32(_mm_slli_epi32(epi32_3, Constant::Int24::LeftShiftFromInt24ToInt32), Constant::Int24::RightShiftFromInt32ToQ31);

			// unpack 32 bit integers to doubles and store
			__m128d pd0 = _mm_cvtepi32_pd(epi32_0);
			__m128d pd1 = _mm_cvtepi32_pd(_mm_shuffle_epi32(epi32_0, Constant::Simd128::PermuteHighAndLow2x64));
			__m128d pd2 = _mm_cvtepi32_pd(epi32_1);
			__m128d pd3 = _mm_cvtepi32_pd(_mm_shuffle_epi32(epi32_1, Constant::Simd128::PermuteHighAndLow2x64));
			__m128d pd4 = _mm_cvtepi32_pd(epi32_2);
			__m128d pd5 = _mm_cvtepi32_pd(_mm_shuffle_epi32(epi32_2, Constant::Simd128::PermuteHighAndLow2x64));
			__m128d pd6 = _mm_cvtepi32_pd(epi32_3);
			__m128d pd7 = _mm_cvtepi32_pd(_mm_shuffle_epi32(epi32_3, Constant::Simd128::PermuteHighAndLow2x64));

			_mm_store_pd(doubles + sample, pd0);
			_mm_store_pd(doubles + sample + 2, pd1);
			_mm_store_pd(doubles + sample + 4, pd2);
			_mm_store_pd(doubles + sample + 6, pd3);
			_mm_store_pd(doubles + sample + 8, pd4);
			_mm_store_pd(doubles + sample + 10, pd5);
			_mm_store_pd(doubles + sample + 12, pd6);
			_mm_store_pd(doubles + sample + 14, pd7);
		}
	}

	void SampleConverter::Q23ToQ31(unsigned __int8* bytes, __int32* int32s, __int32 samples)
	{
		// SSSE3: _mm_shuffle_epi8
		__m128i* source = reinterpret_cast<__m128i*>(bytes);
		__m128i* destination = reinterpret_cast<__m128i*>(int32s);
		for (__int32 destinationIndex = 0, sample = 0, sourceIndex = 0; sample < samples; destinationIndex += 4, sample += 16, sourceIndex += 3)
		{
			// read 16 24 bit integers and unpack to 32 bit
			__m128i epi24_0 = _mm_load_si128(source + sourceIndex);
			__m128i epi24_1 = _mm_load_si128(source + sourceIndex + 1);
			__m128i epi24_2 = _mm_load_si128(source + sourceIndex + 2);

			__m128i epi32_0 = _mm_shuffle_epi8(epi24_0, SampleConverter::ShuffleInt24ToInt32_128);
			__m128i epi32_1 = _mm_shuffle_epi8(_mm_alignr_epi8(epi24_1, epi24_0, 12), SampleConverter::ShuffleInt24ToInt32_128);
			__m128i epi32_2 = _mm_shuffle_epi8(_mm_alignr_epi8(epi24_2, epi24_1, 8), SampleConverter::ShuffleInt24ToInt32_128);
			__m128i epi32_3 = _mm_shuffle_epi8(_mm_alignr_epi8(epi24_2, epi24_2, 4), SampleConverter::ShuffleInt24ToInt32_128);

			// shift from Q23 to Q31 and store 32 bit integers
			// To obtain sign extension (and thereby render negative 24 bit values as negative 32 bit values without conditional upper bit fiddling 
			// logic for sign extension) shift left by eight and then right shift on to the desired [ 0.25, 0.25 ) Q2.21 normalization.
			epi32_0 = _mm_srai_epi32(_mm_slli_epi32(epi32_0, Constant::Int24::LeftShiftFromInt24ToInt32), Constant::Int24::RightShiftFromInt32ToQ31);
			epi32_1 = _mm_srai_epi32(_mm_slli_epi32(epi32_1, Constant::Int24::LeftShiftFromInt24ToInt32), Constant::Int24::RightShiftFromInt32ToQ31);
			epi32_2 = _mm_srai_epi32(_mm_slli_epi32(epi32_2, Constant::Int24::LeftShiftFromInt24ToInt32), Constant::Int24::RightShiftFromInt32ToQ31);
			epi32_3 = _mm_srai_epi32(_mm_slli_epi32(epi32_3, Constant::Int24::LeftShiftFromInt24ToInt32), Constant::Int24::RightShiftFromInt32ToQ31);

			_mm_store_si128(destination + destinationIndex, epi32_0);
			_mm_store_si128(destination + destinationIndex + 1, epi32_1);
			_mm_store_si128(destination + destinationIndex + 2, epi32_2);
			_mm_store_si128(destination + destinationIndex + 3, epi32_3);
		}
	}

	void SampleConverter::Q31ToDouble(__int32* int32s, double* doubles, __int32 samples)
	{
		// SSE2
		__m128i* source = reinterpret_cast<__m128i*>(int32s);
		for (__int32 sample = 0; sample < samples; sample += 4)
		{
			// unpack four 32 bit integers to double and store
			__m128i* block = source + sample / 4;
			__m128i epi32 = _mm_load_si128(block);

			__m128d pd_0 = _mm_cvtepi32_pd(epi32);
			_mm_store_pd(doubles + sample, pd_0);

			__m128d pd_1 = _mm_cvtepi32_pd(_mm_shuffle_epi32(epi32, Constant::Simd128::PermuteHighAndLow2x64));
			_mm_store_pd(doubles + sample + 1, pd_1);
		}
	}

	void SampleConverter::Q31ToQ15(__int32* int32s, __int16* int16s, __int32 samples)
	{
		// SSE2
		__m128i* source = reinterpret_cast<__m128i*>(int32s);
		__m128i* destination = reinterpret_cast<__m128i*>(int16s);
		for (__int32 destinationIndex = 0, sample = 0; sample < samples; ++destinationIndex, sample += 8)
		{
			// pack eight 32 bit integers to 16 bit and store
			__m128i* block = source + sample / 4;
			__m128i epi32_0 = _mm_load_si128(block);
			__m128i epi32_1 = _mm_load_si128(block + 1);
			__m128i epi16 = _mm_packs_epi32(epi32_0, epi32_1);
			_mm_store_si128(destination + destinationIndex, epi16);
		}
	}

	void SampleConverter::Q31ToQ23(__int32* int32s, unsigned __int8* bytes, __int32 samples)
	{
		// SSE4.1: _mm_{max, min}_epi32, SSSE3: _mm_shuffle_epi8
		__m128i* source = reinterpret_cast<__m128i*>(int32s);
		__m128i* destination = reinterpret_cast<__m128i*>(bytes);
		for (__int32 destinationIndex = 0, sample = 0; sample < samples; destinationIndex += 3, sample += 16)
		{
			// load 16 32 bit integers
			__m128i* block = source + sample / 4;
			__m128i epi32_0 = _mm_load_si128(block);
			epi32_0 = _mm_srai_epi32(epi32_0, Constant::ShiftBetween24BitSamplesAndQ31);
			epi32_0 = _mm_min_epi32(SampleConverter::Int24Max128, epi32_0);
			epi32_0 = _mm_max_epi32(SampleConverter::Int24Min128, epi32_0);

			__m128i epi32_1 = _mm_load_si128(block + 1);
			epi32_1 = _mm_srai_epi32(epi32_1, Constant::ShiftBetween24BitSamplesAndQ31);
			epi32_1 = _mm_min_epi32(SampleConverter::Int24Max128, epi32_1);
			epi32_1 = _mm_max_epi32(SampleConverter::Int24Min128, epi32_1);

			__m128i epi32_2 = _mm_load_si128(block + 2);
			epi32_2 = _mm_srai_epi32(epi32_2, Constant::ShiftBetween24BitSamplesAndQ31);
			epi32_2 = _mm_min_epi32(SampleConverter::Int24Max128, epi32_2);
			epi32_2 = _mm_max_epi32(SampleConverter::Int24Min128, epi32_2);

			__m128i epi32_3 = _mm_load_si128(block + 3);
			epi32_3 = _mm_srai_epi32(epi32_3, Constant::ShiftBetween24BitSamplesAndQ31);
			epi32_3 = _mm_min_epi32(SampleConverter::Int24Max128, epi32_3);
			epi32_3 = _mm_max_epi32(SampleConverter::Int24Min128, epi32_3);

			// pack 32 bit integers into 24 bit samples
			epi32_0 = _mm_shuffle_epi8(epi32_0, SampleConverter::ShuffleInt32ToInt24_128);
			epi32_1 = _mm_shuffle_epi8(epi32_1, SampleConverter::ShuffleInt32ToInt24_128);
			epi32_2 = _mm_shuffle_epi8(epi32_2, SampleConverter::ShuffleInt32ToInt24_128);
			epi32_3 = _mm_shuffle_epi8(epi32_3, SampleConverter::ShuffleInt32ToInt24_128);

			__m128i epi24_0 = _mm_or_si128(epi32_0, _mm_slli_si128(epi32_1, 12));
			__m128i epi24_1 = _mm_or_si128(_mm_srli_si128(epi32_1, 4), _mm_slli_si128(epi32_2, 8));
			__m128i epi24_2 = _mm_or_si128(_mm_srli_si128(epi32_2, 8), _mm_slli_si128(epi32_3, 4));

			_mm_store_si128(destination + destinationIndex, epi24_0);
			_mm_store_si128(destination + destinationIndex + 1, epi24_1);
			_mm_store_si128(destination + destinationIndex + 2, epi24_2);
		}
	}
}