#include "stdafx.h"

#include "BiquadDouble.h"

namespace CrossTimeDsp::Dsp
{
	#pragma unmanaged
	BiquadDouble::BiquadDouble(double b0, double b1, double b2, double a0, double a1, double a2, __int32 channels)
	{
		this->channels = channels;
		if (channels == 2)
		{
			this->a1_2 = _mm_set1_pd(a1 / a0);
			this->a2_2 = _mm_set1_pd(a2 / a0);
			this->b0_2 = _mm_set1_pd(b0 / a0);
			this->b1_2 = _mm_set1_pd(b1 / a0);
			this->b2_2 = _mm_set1_pd(b2 / a0);

			this->x1_2 = _mm_setzero_pd();
			this->x2_2 = _mm_setzero_pd();
			this->y1_2 = _mm_setzero_pd();
			this->y2_2 = _mm_setzero_pd();
		}
		else
		{
			this->a1_n = a1 / a0;
			this->a2_n = a2 / a0;
			this->b0_n = b0 / a0;
			this->b1_n = b1 / a0;
			this->b2_n = b2 / a0;

			this->x1_n = new double[channels];
			this->x2_n = new double[channels];
			this->y1_n = new double[channels];
			this->y2_n = new double[channels];
			for (int channel = 0; channel < channels; ++channel)
			{
				this->x1_n[channel] = 0.0;
				this->x2_n[channel] = 0.0;
				this->y1_n[channel] = 0.0;
				this->y2_n[channel] = 0.0;
			}
		}
	}

	BiquadDouble::~BiquadDouble()
	{
		delete this->x1_n;
		delete this->x2_n;
		delete this->y1_n;
		delete this->y2_n;
	}

	void BiquadDouble::Filter(double* block, __int32 offset)
	{
		int maxSample = offset + Constant::FilterBlockSizeInDoubles;
		if (this->channels == 2)
		{
			for (int sample = offset; sample < maxSample; sample += 2)
			{
				__m128d values = _mm_loadu_pd(block + sample);
				__m128d accumulator = _mm_mul_pd(this->b0_2, values);
						accumulator = _mm_add_pd(accumulator, _mm_mul_pd(this->b1_2, this->x1_2));
						accumulator = _mm_add_pd(accumulator, _mm_mul_pd(this->b2_2, this->x2_2));
						accumulator = _mm_sub_pd(accumulator, _mm_mul_pd(this->a1_2, this->y1_2));
						accumulator = _mm_sub_pd(accumulator, _mm_mul_pd(this->a2_2, this->y2_2));
				this->x2_2 = this->x1_2;
				this->x1_2 = values;
				this->y2_2 = this->y1_2;
				this->y1_2 = accumulator;
				_mm_storeu_pd(block + sample, accumulator);
			}
		}
		else
		{
			for (int sample = offset; sample < maxSample; ++sample)
			{
				int channel = sample % this->channels;
				double value = block[sample];
				double accumulator = this->b0_n * value + this->b1_n * this->x1_n[channel] + this->b2_n * this->x2_n[channel] - this->a1_n * this->y1_n[channel] - this->a2_n * this->y2_n[channel];
				this->x2_n[channel] = this->x1_n[channel];
				this->x1_n[channel] = value;
				this->y2_n[channel] = this->y1_n[channel];
				this->y1_n[channel] = accumulator;
				block[sample] = accumulator;
			}
		}
	}

	void BiquadDouble::FilterReverse(double* block, __int32 offset)
	{
		int maxSample = offset + Constant::FilterBlockSizeInDoubles;
		if (this->channels == 2)
		{
			for (int sample = maxSample - 2; sample >= offset; sample -= 2)
			{
				__m128d values = _mm_loadu_pd(block + sample);
				__m128d accumulator = _mm_mul_pd(this->b0_2, values);
						accumulator = _mm_add_pd(accumulator, _mm_mul_pd(this->b1_2, this->x1_2));
						accumulator = _mm_add_pd(accumulator, _mm_mul_pd(this->b2_2, this->x2_2));
						accumulator = _mm_sub_pd(accumulator, _mm_mul_pd(this->a1_2, this->y1_2));
						accumulator = _mm_sub_pd(accumulator, _mm_mul_pd(this->a2_2, this->y2_2));
				this->x2_2 = this->x1_2;
				this->x1_2 = values;
				this->y2_2 = this->y1_2;
				this->y1_2 = accumulator;
				_mm_storeu_pd(block + sample, accumulator);
			}
		}
		else
		{
			for (int sample = maxSample - 1; sample >= offset; --sample)
			{
				int channel = sample % this->channels;
				double value = block[sample];
				double accumulator = this->b0_n * value + this->b1_n * this->x1_n[channel] + this->b2_n * this->x2_n[channel] - this->a1_n * this->y1_n[channel] - this->a2_n * this->y2_n[channel];
				this->x2_n[channel] = this->x1_n[channel];
				this->x1_n[channel] = value;
				this->y2_n[channel] = this->y1_n[channel];
				this->y1_n[channel] = accumulator;
				block[sample] = accumulator;
			}
		}
	}
}