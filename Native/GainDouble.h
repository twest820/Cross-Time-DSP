#pragma once
#include <immintrin.h>
#include "Constant.h"
#include "IFilter.h"

namespace CrossTimeDsp::Dsp
{
	private class GainDouble : IFilter<double>
	{
	private:
		union
		{
			__m128d gain128d;
			__m256d gain256d;
		};

	public:
		GainDouble(double gain);
		~GainDouble();

		virtual void Filter(double* block, __int32 offset);
		virtual void FilterReverse(double* block, __int32 offset);

		void *operator new(size_t size);
		void operator delete(void* gain);
	};
}

