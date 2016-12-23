#include "stdafx.h"
#include "GainM256D.h"

namespace CrossTimeDsp::Dsp
{
	#pragma unmanaged
	GainM256D::GainM256D(double gain)
	{
		this->gain = _mm256_set1_pd(gain);
	}

	void GainM256D::Filter(double* sample, double* maxSample)
	{
		for (; sample < maxSample; sample += 4)
		{
			__m256d values = _mm256_loadu_pd(sample);
			values = _mm256_mul_pd(this->gain, values);
			_mm256_storeu_pd(sample, values);
		}
	}
}