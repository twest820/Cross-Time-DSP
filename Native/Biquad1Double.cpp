#include "stdafx.h"

#include "Biquad1Double.h"

namespace CrossTimeDsp::Dsp
{
	Biquad1Double::Biquad1Double(BiquadCoefficients coefficients, __int32 channels)
	{
		this->channels = channels;
		this->a1 = coefficients.A1;
		this->a2 = coefficients.A2;
		this->b0 = coefficients.B0;
		this->b1 = coefficients.B1;
		this->b2 = coefficients.B2;

		this->x1 = new double[channels];
		this->x2 = new double[channels];
		this->y1 = new double[channels];
		this->y2 = new double[channels];
		for (__int32 channel = 0; channel < channels; ++channel)
		{
			this->x1[channel] = 0.0;
			this->x2[channel] = 0.0;
			this->y1[channel] = 0.0;
			this->y2[channel] = 0.0;
		}
	}

	Biquad1Double::~Biquad1Double()
	{
		delete this->x1;
		delete this->x2;
		delete this->y1;
		delete this->y2;
	}

	void Biquad1Double::Filter(double* block, __int32 offset)
	{
		__int32 maxSample = offset + Constant::FilterBlockSizeInDoubles;
		for (__int32 sample = offset; sample < maxSample; ++sample)
		{
			__int32 channel = sample % this->channels;
			double value = block[sample];
			double accumulator = this->b0 * value + this->b1 * this->x1[channel] + this->b2 * this->x2[channel] - this->a1 * this->y1[channel] - this->a2 * this->y2[channel];
			this->x2[channel] = this->x1[channel];
			this->x1[channel] = value;
			this->y2[channel] = this->y1[channel];
			this->y1[channel] = accumulator;
			block[sample] = accumulator;
		}
	}

	void Biquad1Double::FilterReverse(double* block, __int32 offset)
	{
		__int32 maxSample = offset + Constant::FilterBlockSizeInDoubles;
		for (__int32 sample = maxSample - 1; sample >= offset; --sample)
		{
			__int32 channel = sample % this->channels;
			double value = block[sample];
			double accumulator = this->b0 * value + this->b1 * this->x1[channel] + this->b2 * this->x2[channel] - this->a1 * this->y1[channel] - this->a2 * this->y2[channel];
			this->x2[channel] = this->x1[channel];
			this->x1[channel] = value;
			this->y2[channel] = this->y1[channel];
			this->y1[channel] = accumulator;
			block[sample] = accumulator;
		}
	}
}