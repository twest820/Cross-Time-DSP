#include "stdafx.h"
#include <cmath>
#include <initializer_list>
#include "Constant.h"
#include "FirstOrderFilterQ31_32x64.h"

namespace CrossTimeDsp::Dsp
{
	FirstOrderFilterQ31_32x64::FirstOrderFilterQ31_32x64(double b0, double b1, double a0, double a1, __int32 channels)
	{
		a1 /= a0;
		b0 /= a0;
		b1 /= a0;
		// see remarks in BiquadQ31_32x64..ctor()
		this->postShift = Q31::GetQ31_32x64_PostShift(std::initializer_list<double>({ a1, b0, b1 }));
		double coefficientScaling = std::pow(2.0, -this->postShift);
		this->a1 = new Q31(coefficientScaling * a1, Q31::MaximumFractionalBits);
		this->b0 = new Q31(coefficientScaling * b0, Q31::MaximumFractionalBits);
		this->b1 = new Q31(coefficientScaling * b1, Q31::MaximumFractionalBits);

		this->channels = channels;
		this->x1 = new __int32[channels];
		this->y1 = new __int64[channels];
		for (int channel = 0; channel < channels; ++channel)
		{
			this->x1[channel] = 0;
			this->y1[channel] = 0;
		}
	}

	FirstOrderFilterQ31_32x64::~FirstOrderFilterQ31_32x64()
	{
		delete this->a1;
		delete this->b0;
		delete this->b1;

		delete this->x1;
		delete this->y1;
	}

	void FirstOrderFilterQ31_32x64::Filter(__int32* block, __int32 offset)
	{
		__int32 maxSample = offset + Constant::FilterBlockSizeInInt32s;
		for (__int32 sample = offset; sample < maxSample; ++sample)
		{
			__int32 channel = sample % this->channels;
			__int32 value = block[sample];
			__int64 accumulator = *(this->b0) * value + *(this->b1) * this->x1[channel] - *(this->a1) * this->y1[channel];
			this->x1[channel] = value;
			this->y1[channel] = accumulator << (this->postShift + 1);
			block[sample] = (__int32)(accumulator >> (Q31::MaximumFractionalBits - this->postShift));
		}
	}

	void FirstOrderFilterQ31_32x64::FilterReverse(__int32* block, __int32 offset)
	{
		__int32 maxSample = offset + Constant::FilterBlockSizeInInt32s;
		for (__int32 sample = maxSample - 1; sample >= offset; --sample)
		{
			__int32 channel = sample % this->channels;
			__int32 value = block[sample];
			__int64 accumulator = *(this->b0) * value + *(this->b1) * this->x1[channel] - *(this->a1) * this->y1[channel];
			this->x1[channel] = value;
			this->y1[channel] = accumulator << (this->postShift + 1);
			block[sample] = (__int32)(accumulator >> (Q31::MaximumFractionalBits - this->postShift));
		}
	}
}