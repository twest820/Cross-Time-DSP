#pragma once

namespace CrossTimeDsp::Dsp
{
	template <typename TSample>
	private class IFilter
	{
	public:
		/// <summary>Process the samples from block[offset] to block[offset + Constant.FilterBlockSizeInTSamples].</summary>
		/// <remarks>Processing portions of a <see cref="SampleBlock"/> <see cref="Constant.FilterBlockSizeInBytes"/> bytes at a time amortizes the cost of
		/// the virtual function call to Filter() and offers the compiler a constant set of loop bounds to optimize against.  With good choice of filter
		/// block size this is about 30% faster than per sample operations.</remarks>
		virtual void Filter(TSample* block, __int32 offset) = 0;

		/// <summary>Process the samples from block[offset + Constant.FilterBlockSizeInTSamples] to block[offset] in reverse time.</summary>
		/// <remarks>Same as Filter() but in the opposite direction.</remarks>
		virtual void FilterReverse(TSample* block, __int32 offset) = 0;
	};
}
