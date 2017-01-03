#include "stdafx.h"
#include <cmath>
#include "StereoBiquad1Q31_64x64.h"
#include "Constant.h"
#include "Q31.h"

namespace CrossTimeDsp::Dsp
{
	StereoBiquad1Q31_64x64::StereoBiquad1Q31_64x64(BiquadCoefficients coefficients)
	{
		// scale coefficients into Q0.31 so that state variables can be stored in Q0.63 for maximum precision
		// the post shift is used to undo the scaling at output
		this->postShift = Q63::GetQ31_64x64_PostShift(std::initializer_list<double>({ coefficients.A1, coefficients.A2, coefficients.B0, coefficients.B1, coefficients.B2 }));
		double coefficientScaling = std::pow(2.0, -this->postShift);
		this->a1 = new Q63(coefficientScaling * coefficients.A1, Q63::MaximumFractionalBits);
		this->a2 = new Q63(coefficientScaling * coefficients.A2, Q63::MaximumFractionalBits);
		this->b0 = new Q63(coefficientScaling * coefficients.B0, Q63::MaximumFractionalBits);
		this->b1 = new Q63(coefficientScaling * coefficients.B1, Q63::MaximumFractionalBits);
		this->b2 = new Q63(coefficientScaling * coefficients.B2, Q63::MaximumFractionalBits);

		__int32 channels = 2;
		this->x1 = new __int32[channels];
		this->x2 = new __int32[channels];
		this->y1 = new __int64[channels];
		this->y2 = new __int64[channels];
		for (__int32 channel = 0; channel < channels; ++channel)
		{
			this->x1[channel] = 0;
			this->x2[channel] = 0;
			this->y1[channel] = 0;
			this->y2[channel] = 0;
		}
	}

	StereoBiquad1Q31_64x64::~StereoBiquad1Q31_64x64()
	{
		delete this->a1;
		delete this->a2;
		delete this->b0;
		delete this->b1;
		delete this->b2;

		delete this->x1;
		delete this->x2;
		delete this->y1;
		delete this->y2;
	}

	/// <summary>
	/// Samples must be in [ 0.25, 0.25 ) to guarantee the accumulator does not overflow and wrap around.
	/// </summary>
	void StereoBiquad1Q31_64x64::Filter(__int32* block, __int32 offset)
	{
		__int32 maxSample = offset + Constant::FilterBlockSizeInInt32s;
		for (__int32 sample = offset; sample < maxSample; sample += 2)
		{
			__int32 value0 = block[sample];
			__int64 accumulator0 = *(this->b0) * value0 + *(this->b1) * this->x1[0] + *(this->b2) * this->x2[0] - *(this->a1) * this->y1[0] - *(this->a2) * this->y2[0];
			this->x2[0] = this->x1[0];
			this->x1[0] = value0;
			this->y2[0] = this->y1[0];
			this->y1[0] = accumulator0 << (this->postShift + 1);
			block[sample] = (__int32)(accumulator0 >> (Q31::MaximumFractionalBits - this->postShift));

			__int32 value1 = block[sample + 1];
			__int64 accumulator1 = *(this->b0) * value1 + *(this->b1) * this->x1[1] + *(this->b2) * this->x2[1] - *(this->a1) * this->y1[1] - *(this->a2) * this->y2[1];
			this->x2[1] = this->x1[1];
			this->x1[1] = value1;
			this->y2[1] = this->y1[1];
			this->y1[1] = accumulator1 << (this->postShift + 1);
			block[sample + 1] = (__int32)(accumulator1 >> (Q31::MaximumFractionalBits - this->postShift));
		}
	}

	void StereoBiquad1Q31_64x64::FilterReverse(__int32* block, __int32 offset)
	{
		__int32 maxSample = offset + Constant::FilterBlockSizeInInt32s;
		for (__int32 sample = maxSample - 2; sample >= offset; sample -= 2)
		{
			__int32 value0 = block[sample];
			__int64 accumulator0 = *(this->b0) * value0 + *(this->b1) * this->x1[0] + *(this->b2) * this->x2[0] - *(this->a1) * this->y1[0] - *(this->a2) * this->y2[0];
			this->x2[0] = this->x1[0];
			this->x1[0] = value0;
			this->y2[0] = this->y1[0];
			this->y1[0] = accumulator0 << (this->postShift + 1);
			block[sample] = (__int32)(accumulator0 >> (Q31::MaximumFractionalBits - this->postShift));

			__int32 value1 = block[sample + 1];
			__int64 accumulator1 = *(this->b0) * value1 + *(this->b1) * this->x1[1] + *(this->b2) * this->x2[1] - *(this->a1) * this->y1[1] - *(this->a2) * this->y2[1];
			this->x2[1] = this->x1[1];
			this->x1[1] = value1;
			this->y2[1] = this->y1[1];
			this->y1[1] = accumulator1 << (this->postShift + 1);
			block[sample + 1] = (__int32)(accumulator1 >> (Q31::MaximumFractionalBits - this->postShift));
		}
	}
}
