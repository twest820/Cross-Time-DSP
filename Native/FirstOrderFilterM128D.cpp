#include "stdafx.h"
#include <assert.h>
#include "FirstOrderFilterM128D.h"

namespace CrossTimeDsp::Dsp
{
	#pragma unmanaged
	FirstOrderFilterM128D::FirstOrderFilterM128D(double b0, double b1, double a0, double a1, __int32 channels)
	{
		assert(channels == 2);

		this->a1 = _mm_set1_pd(a1 / a0);
		this->b0 = _mm_set1_pd(b0 / a0);
		this->b1 = _mm_set1_pd(b1 / a0);

		this->channels = channels;
		this->x1 = _mm_setzero_pd();
		this->y1 = _mm_setzero_pd();
	}

	void FirstOrderFilterM128D::Filter(double* sample, double* maxSample)
	{
		for (; sample < maxSample; sample += 2)
		{
			__m128d values = _mm_loadu_pd(sample);
			__m128d accumulator = _mm_mul_pd(this->b0, values);
					accumulator = _mm_add_pd(accumulator, _mm_mul_pd(this->b1, this->x1));
					accumulator = _mm_sub_pd(accumulator, _mm_mul_pd(this->a1, this->y1));
			this->x1 = values;
			this->y1 = accumulator;
			_mm_storeu_pd(sample, accumulator);
		}
	}

	void FirstOrderFilterM128D::FilterReverse(double* sample, double* minSample)
	{
		for (; sample >= minSample; sample -= 2)
		{
			__m128d values = _mm_loadu_pd(sample);
			__m128d accumulator = _mm_mul_pd(this->b0, values);
			accumulator = _mm_add_pd(accumulator, _mm_mul_pd(this->b1, this->x1));
			accumulator = _mm_sub_pd(accumulator, _mm_mul_pd(this->a1, this->y1));
			this->x1 = values;
			this->y1 = accumulator;
			_mm_storeu_pd(sample, accumulator);
		}
	}
}