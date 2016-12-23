#include "stdafx.h"
#include <initializer_list>
#include "Constant.h"
#include "FirstOrderFilterQ31.h"

namespace CrossTimeDsp::Dsp
{
	FirstOrderFilterQ31::FirstOrderFilterQ31(double b0, double b1, double a0, double a1, __int32 channels)
	{
		a1 /= a0;
		b0 /= a0;
		b1 /= a0;
		this->fractionalBitsInCoefficients = Q31::GetOptimalNumberOfFractionalBits(std::initializer_list<double>({ a1, b0, b1 }));
		this->a1 = new Q31(a1, this->fractionalBitsInCoefficients);
		this->b0 = new Q31(b0, this->fractionalBitsInCoefficients);
		this->b1 = new Q31(b1, this->fractionalBitsInCoefficients);

		this->channels = channels;
		this->x1 = new __int32[channels];
		this->y1 = new __int32[channels];
		for (int channel = 0; channel < channels; ++channel)
		{
			this->x1[channel] = 0;
			this->y1[channel] = 0;
		}
	}

	FirstOrderFilterQ31::~FirstOrderFilterQ31()
	{
		delete this->a1;
		delete this->b0;
		delete this->b1;

		delete this->x1;
		delete this->y1;
	}

	void FirstOrderFilterQ31::Filter(__int32* block, __int32 offset)
	{
		__int32 maxSample = offset + Constant::FilterBlockSizeInInt32s;
		for (__int32 sample = offset; sample < maxSample; ++sample)
		{
			__int32 channel = sample % this->channels;
			__int32 value = block[sample];
			__int64 accumulator = *(this->b0) * value + *(this->b1) * this->x1[channel] - *(this->a1) * this->y1[channel];
			this->x1[channel] = value;
			this->y1[channel] = (__int32)(accumulator >> this->fractionalBitsInCoefficients);
			block[sample] = this->y1[channel];
		}
	}

	void FirstOrderFilterQ31::FilterReverse(__int32* block, __int32 offset)
	{
		__int32 maxSample = offset + Constant::FilterBlockSizeInInt32s;
		for (__int32 sample = maxSample - 1; sample >= offset; --sample)
		{
			__int32 channel = sample % this->channels;
			__int32 value = block[sample];
			__int64 accumulator = *(this->b0) * value + *(this->b1) * this->x1[channel] - *(this->a1) * this->y1[channel];
			this->x1[channel] = value;
			this->y1[channel] = (__int32)(accumulator >> this->fractionalBitsInCoefficients);
			block[sample] = this->y1[channel];
		}
	}
}