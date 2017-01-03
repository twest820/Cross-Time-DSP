#pragma once
#include "FilterTypeNative.h"

namespace CrossTimeDsp::Dsp
{
	public enum class FilterType : __int32
	{
		Allpass = static_cast<__int32>(FilterTypeNative::Allpass),
		Bandpass = static_cast<__int32>(FilterTypeNative::Bandpass),
		Highpass = static_cast<__int32>(FilterTypeNative::Highpass),
		HighShelf = static_cast<__int32>(FilterTypeNative::HighShelf),
		Lowpass = static_cast<__int32>(FilterTypeNative::Lowpass),
		LowShelf = static_cast<__int32>(FilterTypeNative::LowShelf),
		Notch = static_cast<__int32>(FilterTypeNative::Notch),
		Peaking = static_cast<__int32>(FilterTypeNative::Peaking)
	};
}

