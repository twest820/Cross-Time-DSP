#include "stdafx.h"
#include "BiquadDouble.h"
#include "BiquadQ31.h"
#include "BiquadQ31_32x64.h"
#include "BiquadQ31_64x64.h"
#include "Constant.h"
#include "FilterBank.h"
#include "FirstOrderFilterDouble.h"
#include "FirstOrderFilterQ31.h"
#include "FirstOrderFilterQ31_32x64.h"
#include "GainDouble.h"
#include "GainQ31.h"
#include "GainQ31_64x64.h"

using namespace System;

namespace CrossTimeDsp::Dsp
{
	FilterBank::FilterBank(FilterPrecision precision, double q31_32x64_Threshold, double q31_64x64_Threshold)
	{
		this->doubleFilters = new std::vector<IFilter<double>*>();
		this->intFilters = new std::vector<IFilter<__int32>*>();
		this->precision = precision;
		this->q31_32x64_Threshold = q31_32x64_Threshold;
		this->q31_64x64_Threshold = q31_64x64_Threshold;
	}

	FilterBank::~FilterBank()
	{
		for each (IFilter<double>* filter in *(this->doubleFilters))
		{
			delete filter;
		}
		delete this->doubleFilters;

		for each (IFilter<__int32>* filter in *(this->intFilters))
		{
			delete filter;
		}
		delete this->intFilters;
	}

	void FilterBank::AddBiquad(FilterType type, __int32 fs, double f0, double gainInDB, double q, __int32 channels)
	{
		if (this->precision == FilterPrecision::Double)
		{
			this->doubleFilters->push_back(this->CreateBiquad<double>(type, fs, f0, gainInDB, q, channels));
		}
		else
		{
			this->intFilters->push_back(this->CreateBiquad<__int32>(type, fs, f0, gainInDB, q, channels));
		}
	}

	void FilterBank::AddFirstOrderFilter(FilterType type, __int32 fs, double f0, __int32 channels)
	{
		if (this->precision == FilterPrecision::Double)
		{
			this->doubleFilters->push_back(this->CreateFirstOrder<double>(type, fs, f0, channels));
		}
		else
		{
			this->intFilters->push_back(this->CreateFirstOrder<__int32>(type, fs, f0, channels));
		}
	}

	void FilterBank::AddGain(double gainInDb)
	{
		if (this->precision == FilterPrecision::Double)
		{
			this->doubleFilters->push_back(this->CreateGain<double>(gainInDb));
		}
		else
		{
			this->intFilters->push_back(this->CreateGain<__int32>(gainInDb));
		}
	}

	template <typename TSample>	IFilter<TSample>* FilterBank::CreateAllpassBiquad(double w0, double alpha, FilterPrecision precision, __int32 channels)
	{
		double b0 = 1.0 - alpha;
		double b1 = -2.0 * Math::Cos(w0);
		double b2 = 1.0 + alpha;
		double a0 = 1.0 + alpha;
		double a1 = -2.0 * Math::Cos(w0);
		double a2 = 1.0 - alpha;
		return this->CreateBiquad<TSample>(b0, b1, b2, a0, a1, a2, precision, channels);
	}

	template <typename TSample>	IFilter<TSample>* FilterBank::CreateAllpassFirstOrder(double w0, FilterPrecision precision, __int32 channels)
	{
		double c = (Math::Tan(w0 / 2.0) - 1.0) / (Math::Tan(w0 / 2.0) + 1.0);
		double b0 = c;
		double b1 = 1.0;
		double a0 = 1.0;
		double a1 = c;
		return this->CreateFirstOrder<TSample>(b0, b1, a0, a1, precision, channels);
	}

	template <typename TSample>	IFilter<TSample>* FilterBank::CreateBandpass(double w0, double alpha, FilterPrecision precision, __int32 channels)
	{
		double b0 = alpha;
		double b1 = 0.0;
		double b2 = -alpha;
		double a0 = 1.0 + alpha;
		double a1 = -2.0 * Math::Cos(w0);
		double a2 = 1.0 - alpha;
		return this->CreateBiquad<TSample>(b0, b1, b2, a0, a1, a2, precision, channels);
	}

	template <typename TSample>	IFilter<TSample>* FilterBank::CreateBiquad(FilterType type, __int32 fs, double f0, double gainInDB, double q, __int32 channels)
	{
		FilterPrecision precision = this->MaybeResolveAdaptiveFilterPrecision(fs, f0);
		double w0 = this->GetW0(fs, f0);
		double alpha = this->GetAlpha(w0, q);
		switch (type)
		{
		case FilterType::Allpass:
			return this->CreateAllpassBiquad<TSample>(w0, alpha, precision, channels);
		case FilterType::Bandpass:
			return this->CreateBandpass<TSample>(w0, alpha, precision, channels);
		case FilterType::Highpass:
			return this->CreateHighpassBiquad<TSample>(w0, alpha, precision, channels);
		case FilterType::HighShelf:
			return this->CreateHighShelf<TSample>(w0, alpha, gainInDB, precision, channels);
		case FilterType::Lowpass:
			return this->CreateLowpassBiquad<TSample>(w0, alpha, precision, channels);
		case FilterType::LowShelf:
			return this->CreateLowShelf<TSample>(w0, alpha, gainInDB, precision, channels);
		case FilterType::Notch:
			return this->CreateNotch<TSample>(w0, alpha, precision, channels);
		case FilterType::Peaking:
			return this->CreatePeaking<TSample>(w0, alpha, gainInDB, precision, channels);
		default:
			throw gcnew NotSupportedException(String::Format("Unhandled filter type {0}.", type));
		}
	}

	template <typename TSample> IFilter<TSample>* FilterBank::CreateBiquad(double b0, double b1, double b2, double a0, double a1, double a2, FilterPrecision precision, __int32 channels)
	{
		switch (precision)
		{
		case FilterPrecision::Double:
			return (IFilter<TSample>*)new BiquadDouble(b0, b1, b2, a0, a1, a2, channels);
		case FilterPrecision::Q31:
			return (IFilter<TSample>*)new BiquadQ31(b0, b1, b2, a0, a1, a2, channels);
		case FilterPrecision::Q31_32x64:
			return (IFilter<TSample>*)new BiquadQ31_32x64(b0, b1, b2, a0, a1, a2, channels);
		case FilterPrecision::Q31_64x64:
			return (IFilter<TSample>*)new BiquadQ31_64x64(b0, b1, b2, a0, a1, a2, channels);
		default:
			throw gcnew NotSupportedException(String::Format("Unhandled filter precision {0}.", precision));
		}
	}

	template <typename TSample>	IFilter<TSample>* FilterBank::CreateFirstOrder(FilterType type, __int32 fs, double f0, __int32 channels)
	{
		FilterPrecision precision = this->MaybeResolveAdaptiveFilterPrecision(fs, f0);
		double w0 = this->GetW0(fs, f0);
		switch (type)
		{
		case FilterType::Allpass:
			return this->CreateAllpassFirstOrder<TSample>(w0, precision, channels);
		default:
			throw gcnew NotSupportedException(String::Format("Unhandled filter type {0}.", type));
		}
	}

	template <typename TSample>	IFilter<TSample>* FilterBank::CreateFirstOrder(double b0, double b1, double a0, double a1, FilterPrecision precision, __int32 channels)
	{
		switch (precision)
		{
		case FilterPrecision::Double:
			return (IFilter<TSample>*)new FirstOrderFilterDouble(b0, b1, a0, a1, channels);
		case FilterPrecision::Q31:
			return (IFilter<TSample>*)new FirstOrderFilterQ31(b0, b1, a0, a1, channels);
		case FilterPrecision::Q31_32x64:
		case FilterPrecision::Q31_64x64:
			return (IFilter<TSample>*)new FirstOrderFilterQ31_32x64(b0, b1, a0, a1, channels);
		default:
			throw gcnew NotSupportedException(String::Format("Unhandled filter precision {0}.", precision));
		}
	}

	template <typename TSample>	IFilter<TSample>* FilterBank::CreateGain(double gainInDB)
	{
		double gain = Math::Pow(10.0, gainInDB / 20.0);
		switch (this->precision)
		{
		case FilterPrecision::Double:
			return (IFilter<TSample>*)new GainDouble(gain);
		case FilterPrecision::Q31:
		case FilterPrecision::Q31_32x64:
		case FilterPrecision::Q31Adaptive:
			return (IFilter<TSample>*)new GainQ31(gain);
		case FilterPrecision::Q31_64x64:
			return (IFilter<TSample>*)new GainQ31_64x64(gain);
		default:
			throw gcnew NotSupportedException(String::Format("Unhandled filter precision {0}.", this->precision));
		}
	}

	template <typename TSample>	IFilter<TSample>* FilterBank::CreateHighpassBiquad(double w0, double alpha, FilterPrecision precision, __int32 channels)
	{
		double b0 = (1.0 + Math::Cos(w0)) / 2.0;
		double b1 = -(1.0 + Math::Cos(w0));
		double b2 = (1.0 + Math::Cos(w0)) / 2.0;
		double a0 = 1.0 + alpha;
		double a1 = -2.0 * Math::Cos(w0);
		double a2 = 1.0 - alpha;
		return this->CreateBiquad<TSample>(b0, b1, b2, a0, a1, a2, precision, channels);
	}

	template <typename TSample>	IFilter<TSample>* FilterBank::CreateHighShelf(double w0, double alpha, double gainInDB, FilterPrecision precision, __int32 channels)
	{
		double a = this->GetA(gainInDB);
		double b0 = a * ((a + 1.0) + (a - 1.0) * Math::Cos(w0) + 2.0 * Math::Sqrt(a) * alpha);
		double b1 = -2.0 * a * ((a - 1.0) + (a + 1.0) * Math::Cos(w0));
		double b2 = a * ((a + 1.0) + (a - 1.0) * Math::Cos(w0) - 2.0 * Math::Sqrt(a) * alpha);
		double a0 = (a + 1.0) - (a - 1.0) * Math::Cos(w0) + 2.0 * Math::Sqrt(a) * alpha;
		double a1 = 2.0 * ((a - 1.0) - (a + 1.0) * Math::Cos(w0));
		double a2 = (a + 1.0) - (a - 1.0) * Math::Cos(w0) - 2.0 * Math::Sqrt(a) * alpha;
		return this->CreateBiquad<TSample>(b0, b1, b2, a0, a1, a2, precision, channels);
	}

	template <typename TSample>	IFilter<TSample>* FilterBank::CreateLowShelf(double w0, double alpha, double gainInDB, FilterPrecision precision, __int32 channels)
	{
		double a = this->GetA(gainInDB);
		double b0 = a * ((a + 1.0) - (a - 1.0) * Math::Cos(w0) + 2.0 * Math::Sqrt(a) * alpha);
		double b1 = 2.0 * a * ((a - 1.0) - (a + 1.0) * Math::Cos(w0));
		double b2 = a * ((a + 1.0) - (a - 1.0) * Math::Cos(w0) - 2.0 * Math::Sqrt(a) * alpha);
		double a0 = (a + 1.0) + (a - 1.0) * Math::Cos(w0) + 2.0 * Math::Sqrt(a) * alpha;
		double a1 = -2.0 * ((a - 1.0) + (a + 1.0) * Math::Cos(w0));
		double a2 = (a + 1.0) + (a - 1.0) * Math::Cos(w0) - 2.0 * Math::Sqrt(a) * alpha;
		return this->CreateBiquad<TSample>(b0, b1, b2, a0, a1, a2, precision, channels);
	}

	template <typename TSample>	IFilter<TSample>* FilterBank::CreateLowpassBiquad(double w0, double alpha, FilterPrecision precision, __int32 channels)
	{
		double b0 = (1.0 - Math::Cos(w0)) / 2.0;
		double b1 = 1.0 - Math::Cos(w0);
		double b2 = (1.0 - Math::Cos(w0)) / 2.0;
		double a0 = 1.0 + alpha;
		double a1 = -2.0 * Math::Cos(w0);
		double a2 = 1.0 - alpha;
		return this->CreateBiquad<TSample>(b0, b1, b2, a0, a1, a2, precision, channels);
	}

	template <typename TSample>	IFilter<TSample>* FilterBank::CreateNotch(double w0, double alpha, FilterPrecision precision, __int32 channels)
	{
		double b0 = 1.0;
		double b1 = -2.0 * Math::Cos(w0);
		double b2 = 1.0;
		double a0 = 1.0 + alpha;
		double a1 = -2.0 * Math::Cos(w0);
		double a2 = 1.0 - alpha;
		return this->CreateBiquad<TSample>(b0, b1, b2, a0, a1, a2, precision, channels);
	}

	template <typename TSample>	IFilter<TSample>* FilterBank::CreatePeaking(double w0, double alpha, double gainInDB, FilterPrecision precision, __int32 channels)
	{
		double a = this->GetA(gainInDB);
		double b0 = 1.0 + alpha * a;
		double b1 = -2.0 * Math::Cos(w0);
		double b2 = 1.0 - alpha * a;
		double a0 = 1.0 + alpha / a;
		double a1 = -2.0 * Math::Cos(w0);
		double a2 = 1.0 - alpha / a;
		return this->CreateBiquad<TSample>(b0, b1, b2, a0, a1, a2, precision, channels);
	}

	SampleBlock^ FilterBank::Filter(SampleBlock^ block, SampleType dataPathSampleType, SampleType outputSampleType)
	{
		block = block->ConvertTo(dataPathSampleType);
		pin_ptr<double> blockAsDoubles = nullptr;
		pin_ptr<__int32> blockAsInt32s = nullptr;
		switch (dataPathSampleType)
		{
		case SampleType::Double:
			blockAsDoubles = &block->DoubleBuffer[0];
			this->Filter<double>(this->doubleFilters, blockAsDoubles, block->DoubleBufferCount, Constant::FilterBlockSizeInDoubles);
			break;
		case SampleType::Int32:
			blockAsInt32s = &block->IntBuffer[0];
			this->Filter<__int32>(this->intFilters, blockAsInt32s, block->IntBufferCount, Constant::FilterBlockSizeInInt32s);
			break;
		default:
			throw gcnew NotSupportedException(String::Format("Unhandled data path sample type {0}.", dataPathSampleType));
		}

		return block->ConvertTo(outputSampleType);
	}

	template <typename TSample>	void FilterBank::Filter(std::vector<IFilter<TSample>*>* filters, TSample* block, __int32 blockLength, __int32 filterBlockSizeInSamples)
	{
		for (__int32 index = 0; index < blockLength; index += filterBlockSizeInSamples)
		{
			for (__int32 filter = 0; filter < filters->size(); ++filter)
			{
				filters->at(filter)->Filter(block, index);
			}
		}
	}

	SampleBlock^ FilterBank::FilterReverse(SampleBlock^ block, SampleType dataPathSampleType, SampleType outputSampleType)
	{
		block = block->ConvertTo(dataPathSampleType);
		pin_ptr<double> blockAsDoubles = nullptr;
		pin_ptr<__int32> blockAsInt32s = nullptr;
		__int32 count = 0;
		switch (dataPathSampleType)
		{
		case SampleType::Double:
			count = block->DoubleBufferCount;
			if (count % Constant::FilterBlockSizeInDoubles != 0)
			{
				count = (count / Constant::FilterBlockSizeInDoubles + 1) * Constant::FilterBlockSizeInDoubles;
			}
			Debug::Assert(count * sizeof(double) <= block->MaximumSizeInBytes, String::Format("Block allocation failure: maximum size of {0} bytes is not a multiple of the {1} byte filter block size.", block->MaximumSizeInBytes, Constant::FilterBlockSizeInBytes));
			blockAsDoubles = &block->DoubleBuffer[0];
			this->FilterReverse<double>(this->doubleFilters, blockAsDoubles, count, Constant::FilterBlockSizeInDoubles);
			break;
		case SampleType::Int32:
			count = block->IntBufferCount;
			if (count % Constant::FilterBlockSizeInInt32s != 0)
			{
				count = (count / Constant::FilterBlockSizeInInt32s + 1) * Constant::FilterBlockSizeInInt32s;
			}
			Debug::Assert(count * sizeof(__int32) <= block->MaximumSizeInBytes, String::Format("Block allocation failure: maximum size of {0} bytes is not a multiple of the {1} byte filter block size.", block->MaximumSizeInBytes, Constant::FilterBlockSizeInBytes));
			blockAsInt32s = &block->IntBuffer[0];
			this->FilterReverse<__int32>(this->intFilters, blockAsInt32s, count, Constant::FilterBlockSizeInInt32s);
			break;
		default:
			throw gcnew NotSupportedException(String::Format("Unhandled data path sample type {0}.", dataPathSampleType));
		}

		return block->ConvertTo(outputSampleType);
	}

	template <typename TSample>	void FilterBank::FilterReverse(std::vector<IFilter<TSample>*>* filters, TSample* block, __int32 count, __int32 filterBlockSizeInSamples)
	{
		for (__int32 index = count - filterBlockSizeInSamples; index >= 0; index -= filterBlockSizeInSamples)
		{
			for (__int32 filter = 0; filter < filters->size(); ++filter)
			{
				filters->at(filter)->FilterReverse(block, index);
			}
		}
	}

	double FilterBank::GetA(double gainInDB)
	{
		return Math::Pow(10.0, gainInDB / 40.0);
	}

	double FilterBank::GetAlpha(double w0, double q)
	{
		return Math::Sin(w0) / (2.0 * q);
	}

	double FilterBank::GetW0(double fs, double f0)
	{
		return 2.0 * Math::PI * f0 / fs;
	}

	FilterPrecision FilterBank::MaybeResolveAdaptiveFilterPrecision(double fs, double f0)
	{
		if (this->precision != FilterPrecision::Q31Adaptive)
		{
			return this->precision;
		}

		double normalizedCenterFrequency = f0 / fs;
		if (normalizedCenterFrequency > this->q31_32x64_Threshold)
		{
			return FilterPrecision::Q31;
		}
		else if (normalizedCenterFrequency > this->q31_64x64_Threshold)
		{
			return FilterPrecision::Q31_32x64;
		}
		else
		{
			return FilterPrecision::Q31_64x64;
		}
	}
}