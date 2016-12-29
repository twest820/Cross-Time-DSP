#include "stdafx.h"
#include <cmath>
#include <initializer_list>
#include "Constant.h"
#include "FirstOrder1Q31_32x64.h"

namespace CrossTimeDsp::Dsp
{
	FirstOrder1Q31_32x64::FirstOrder1Q31_32x64(FirstOrderCoefficients coefficients, __int32 channels)
	{
		// see remarks in BiquadQ31_32x64..ctor()
		this->postShift = Q31::GetQ31_32x64_PostShift(std::initializer_list<double>({ coefficients.A1, coefficients.B0, coefficients.B1 }));
		double coefficientScaling = std::pow(2.0, -this->postShift);
		this->a1 = new Q31(coefficientScaling * coefficients.A1, Q31::MaximumFractionalBits);
		this->b0 = new Q31(coefficientScaling * coefficients.B0, Q31::MaximumFractionalBits);
		this->b1 = new Q31(coefficientScaling * coefficients.B1, Q31::MaximumFractionalBits);

		this->channels = channels;
		this->x1 = new __int32[channels];
		this->y1 = new __int64[channels];
		for (__int32 channel = 0; channel < channels; ++channel)
		{
			this->x1[channel] = 0;
			this->y1[channel] = 0;
		}
	}

	FirstOrder1Q31_32x64::~FirstOrder1Q31_32x64()
	{
		delete this->a1;
		delete this->b0;
		delete this->b1;

		delete this->x1;
		delete this->y1;
	}

	void FirstOrder1Q31_32x64::Filter(__int32* block, __int32 offset)
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

	void FirstOrder1Q31_32x64::FilterReverse(__int32* block, __int32 offset)
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