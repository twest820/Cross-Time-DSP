#include "stdafx.h"
#include <new>
#include "FirstOrder1Double.h"

namespace CrossTimeDsp::Dsp
{
	FirstOrder1Double::FirstOrder1Double(FirstOrderCoefficients coefficients, __int32 channels)
	{
		this->channels = channels;
		this->a1 = coefficients.A1;
		this->b0 = coefficients.B0;
		this->b1 = coefficients.B1;

		this->x1 = new double[channels];
		this->y1 = new double[channels];
		for (__int32 channel = 0; channel < channels; ++channel)
		{
			this->x1[channel] = 0.0;
			this->y1[channel] = 0.0;
		}
	}

	FirstOrder1Double::~FirstOrder1Double()
	{
		delete this->x1;
		delete this->y1;
	}

	void FirstOrder1Double::Filter(double* block, __int32 offset)
	{
		__int32 maxSample = offset + Constant::FilterBlockSizeInDoubles;
		for (__int32 sample = offset; sample < maxSample; ++sample)
		{
			__int32 channel = sample % this->channels;
			double value = block[sample];
			double accumulator = this->b0 * value + this->b1 * this->x1[channel] - this->a1 * this->y1[channel];
			this->x1[channel] = value;
			this->y1[channel] = accumulator;
			block[sample] = accumulator;
		}
	}

	void FirstOrder1Double::FilterReverse(double* block, __int32 offset)
	{
		__int32 maxSample = offset + Constant::FilterBlockSizeInDoubles;
		for (__int32 sample = maxSample - 1; sample >= offset; --sample)
		{
			__int32 channel = sample % this->channels;
			double value = block[sample];
			double accumulator = this->b0 * value + this->b1 * this->x1[channel] - this->a1 * this->y1[channel];
			this->x1[channel] = value;
			this->y1[channel] = accumulator;
			block[sample] = accumulator;
		}
	}
}