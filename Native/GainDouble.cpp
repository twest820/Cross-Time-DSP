#include "stdafx.h"
#include <new>
#include "GainDouble.h"

namespace CrossTimeDsp::Dsp
{
	#pragma unmanaged
	GainDouble::GainDouble(double gain)
	{
		this->gain = _mm_set1_pd(gain);
	}

	void GainDouble::Filter(double* block, __int32 offset)
	{
		int maxSample = offset + Constant::FilterBlockSizeInDoubles;
		for (int sample = offset; sample < maxSample; sample += 2)
		{
			__m128d values = _mm_loadu_pd(block + sample);
					values = _mm_mul_pd(this->gain, values);
			_mm_storeu_pd(block + sample, values);
		}
	}

	void GainDouble::FilterReverse(double* block, __int32 offset)
	{
		int maxSample = offset + Constant::FilterBlockSizeInDoubles;
		for (int sample = maxSample - 2; sample >= offset; sample -= 2)
		{
			__m128d values = _mm_loadu_pd(block + sample);
					values = _mm_mul_pd(this->gain, values);
			_mm_storeu_pd(block + sample, values);
		}
	}
}