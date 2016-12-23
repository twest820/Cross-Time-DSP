#include "stdafx.h"
#include "Constant.h"
#include "GainQ31_64x64.h"

namespace CrossTimeDsp::Dsp
{
	GainQ31_64x64::GainQ31_64x64(double scaleFactor)
	{
		this->scaleFactor = new Q63(scaleFactor);
	}

	GainQ31_64x64::~GainQ31_64x64()
	{
		delete this->scaleFactor;
	}

	void GainQ31_64x64::Filter(__int32* block, __int32 offset)
	{
		__int32 maxSample = offset + Constant::FilterBlockSizeInInt32s;
		for (__int32 sample = offset; sample < maxSample; ++sample)
		{
			block[sample] = (__int32)((*(this->scaleFactor) * block[sample]) >> this->scaleFactor->FractionalBits());
		}
	}

	void GainQ31_64x64::FilterReverse(__int32* block, __int32 offset)
	{
		this->Filter(block, offset);
	}
}