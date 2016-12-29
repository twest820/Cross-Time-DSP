#include "stdafx.h"
#include <new>
#include "GainDouble.h"
#include "InstructionSet.h"

namespace CrossTimeDsp::Dsp
{
	#pragma unmanaged
	GainDouble::GainDouble(double gain)
	{
		if (InstructionSet::Avx() && Constant::Simd256FilteringEnabled)
		{
			this->gain256d = _mm256_set1_pd(gain);
			_mm256_zeroupper();
		}
		else
		{
			this->gain128d = _mm_set1_pd(gain);
		}
	}

	GainDouble::~GainDouble()
	{
		// nothing to do
	}

	void GainDouble::Filter(double* block, __int32 offset)
	{
		__int32 maxSample = offset + Constant::FilterBlockSizeInDoubles;
		if (InstructionSet::Avx() && Constant::Simd256FilteringEnabled)
		{
			for (__int32 sample = offset; sample < maxSample; sample += 4)
			{
				__m256d values = _mm256_load_pd(block + sample);
				values = _mm256_mul_pd(this->gain256d, values);
				_mm256_store_pd(block + sample, values);
			}
			_mm256_zeroupper();
		}
		else
		{
			for (__int32 sample = offset; sample < maxSample; sample += 2)
			{
				__m128d values = _mm_load_pd(block + sample);
				values = _mm_mul_pd(this->gain128d, values);
				_mm_store_pd(block + sample, values);
			}
		}
	}

	void GainDouble::FilterReverse(double* block, __int32 offset)
	{
		__int32 maxSample = offset + Constant::FilterBlockSizeInDoubles;
		if (InstructionSet::Avx() && Constant::Simd256FilteringEnabled)
		{
			for (__int32 sample = maxSample - 4; sample >= offset; sample -= 4)
			{
				__m256d values = _mm256_load_pd(block + sample);
				values = _mm256_mul_pd(this->gain256d, values);
				_mm256_store_pd(block + sample, values);
			}
			_mm256_zeroupper();
		}
		else
		{
			for (__int32 sample = maxSample - 2; sample >= offset; sample -= 2)
			{
				__m128d values = _mm_load_pd(block + sample);
				values = _mm_mul_pd(this->gain128d, values);
				_mm_store_pd(block + sample, values);
			}
		}
	}

	void *GainDouble::operator new(size_t size)
	{
		return _aligned_malloc(size, alignof(__m256d));
	}

	void GainDouble::operator delete(void* gain)
	{
		_aligned_free(gain);
	}
}