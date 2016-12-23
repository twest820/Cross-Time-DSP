#pragma once
#include <vector>
#include "FilterPrecision.h"
#include "FilterType.h"
#include "IFilter.h"
#include "SampleBlock.h"
#include "SampleType.h"

using namespace System;

namespace CrossTimeDsp::Dsp
{
	public ref class FilterBank
	{
	private:
		std::vector<IFilter<double>*>* doubleFilters;
		std::vector<IFilter<__int32>*>* intFilters;
		initonly FilterPrecision precision;
		initonly double q31_32x64_Threshold;
		initonly double q31_64x64_Threshold;

	public:
		FilterBank(FilterPrecision precision, double q31_32x64_Threshold, double q31_64x64_Threshold);
		~FilterBank();

		property __int32 FilterCount
		{
			__int32 get() { return (__int32)(this->doubleFilters->size() + this->intFilters->size()); }
		}

		void AddBiquad(FilterType type, __int32 fs, double f0, double gainInDB, double q, __int32 channels);
		void AddFirstOrderFilter(FilterType type, __int32 fs, double f0, __int32 channels);
		void AddGain(double gainInDb);
		SampleBlock^ Filter(SampleBlock^ block, SampleType dataPathSampleType, SampleType outputSampleType);
		SampleBlock^ FilterReverse(SampleBlock^ block, SampleType dataPathSampleType, SampleType outputSampleType);

	private:
		template <typename TSample> IFilter<TSample>* CreateAllpassBiquad(double w0, double alpha, FilterPrecision precision, __int32 channels);
		template <typename TSample> IFilter<TSample>* CreateAllpassFirstOrder(double w0, FilterPrecision precision, __int32 channels);
		template <typename TSample> IFilter<TSample>* CreateBandpass(double w0, double alpha, FilterPrecision precision, __int32 channels);
		template <typename TSample> IFilter<TSample>* CreateBiquad(FilterType type, __int32 fs, double f0, double gainInDB, double q, __int32 channels);
		template <typename TSample> IFilter<TSample>* CreateBiquad(double b0, double b1, double b2, double a0, double a1, double a2, FilterPrecision precision, __int32 channels);
		template <typename TSample> IFilter<TSample>* CreateFirstOrder(FilterType type, __int32 fs, double f0, __int32 channels);
		template <typename TSample> IFilter<TSample>* CreateFirstOrder(double b0, double b1, double a0, double a1, FilterPrecision precision, __int32 channels);
		template <typename TSample> IFilter<TSample>* CreateGain(double gainInDB);
		template <typename TSample> IFilter<TSample>* CreateHighpassBiquad(double w0, double alpha, FilterPrecision precision, __int32 channels);
		template <typename TSample> IFilter<TSample>* CreateHighShelf(double w0, double alpha, double gainInDB, FilterPrecision precision, __int32 channels);
		template <typename TSample> IFilter<TSample>* CreateLowShelf(double w0, double alpha, double gainInDB, FilterPrecision precision, __int32 channels);
		template <typename TSample> IFilter<TSample>* CreateLowpassBiquad(double w0, double alpha, FilterPrecision precision, __int32 channels);
		template <typename TSample> IFilter<TSample>* CreateNotch(double w0, double alpha, FilterPrecision precision, __int32 channels);
		template <typename TSample> IFilter<TSample>* CreatePeaking(double w0, double alpha, double gainInDB, FilterPrecision precision, __int32 channels);
		
		template <typename TSample> void Filter(std::vector<IFilter<TSample>*>* filters, TSample* block, __int32 blockLength, __int32 filterBlockSizeInSamples);
		template <typename TSample> void FilterReverse(std::vector<IFilter<TSample>*>* filters, TSample* block, __int32 count, __int32 filterBlockSizeInSamples);

		double GetA(double gainInDB);
		double GetAlpha(double w0, double q);
		double GetW0(double fs, double f0);
		FilterPrecision MaybeResolveAdaptiveFilterPrecision(double fs, double f0);
	};
}

