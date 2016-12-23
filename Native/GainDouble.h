#pragma once
#include <immintrin.h>
#include "Constant.h"
#include "IFilter.h"

using namespace System;
using namespace System::Diagnostics;

namespace CrossTimeDsp::Dsp
{
	private class GainDouble : IFilter<double>
	{
	private:
		__m128d gain;

	public:
		GainDouble(double gain);
		~GainDouble();

		virtual void Filter(double* block, __int32 offset);
		virtual void FilterReverse(double* block, __int32 offset);
	};
}

