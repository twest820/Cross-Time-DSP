#include "stdafx.h"
#include <assert.h>
#include "BiquadM256D.h"

namespace CrossTimeDsp::Dsp
{
#pragma unmanaged
	BiquadM256D::BiquadM256D(double b0, double b1, double b2, double a0, double a1, double a2, __int32 channels)
	{
		assert(channels == 2);

		this->a1 = _mm_set1_pd(a1 / a0);
		this->a2 = _mm_set1_pd(a2 / a0);
		this->b0 = _mm_set1_pd(b0 / a0);
		this->b1 = _mm_set1_pd(b1 / a0);
		this->b2 = _mm_set1_pd(b2 / a0);

		this->channels = channels;
		this->x1 = _mm_setzero_pd();
		this->x2 = _mm_setzero_pd();
		this->y1 = _mm_setzero_pd();
		this->y2 = _mm_setzero_pd();
	}

	void BiquadM256D::Filter(double* sample, double* maxSample)
	{
		for (; sample < maxSample; sample += 2)
		{
			__m128d values = _mm_loadu_pd(sample);
			__m128d accumulator = _mm_mul_pd(this->b0, values);
					accumulator = _mm_add_pd(accumulator, _mm_mul_pd(this->b1, this->x1));
					accumulator = _mm_add_pd(accumulator, _mm_mul_pd(this->b2, this->x2));
					accumulator = _mm_sub_pd(accumulator, _mm_mul_pd(this->a1, this->y1));
					accumulator = _mm_sub_pd(accumulator, _mm_mul_pd(this->a2, this->y2));
			this->x2 = this->x1;
			this->x1 = values;
			this->y2 = this->y1;
			this->y1 = accumulator;
			_mm_storeu_pd(sample, accumulator);
		}
	}

	void BiquadM256D::FilterReverse(double* sample, double* minSample)
	{
		for (; sample >= minSample; sample -= 2)
		{
			__m128d values = _mm_loadu_pd(sample);
			__m128d accumulator = _mm_mul_pd(this->b0, values);
					accumulator = _mm_add_pd(accumulator, _mm_mul_pd(this->b1, this->x1));
					accumulator = _mm_add_pd(accumulator, _mm_mul_pd(this->b2, this->x2));
					accumulator = _mm_sub_pd(accumulator, _mm_mul_pd(this->a1, this->y1));
					accumulator = _mm_sub_pd(accumulator, _mm_mul_pd(this->a2, this->y2));
			this->x2 = this->x1;
			this->x1 = values;
			this->y2 = this->y1;
			this->y1 = accumulator;
			_mm_storeu_pd(sample, accumulator);
		}
	}
}