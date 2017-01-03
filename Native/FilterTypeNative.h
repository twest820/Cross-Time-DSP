#pragma once
namespace CrossTimeDsp::Dsp
{
	enum class __declspec(dllexport) FilterTypeNative : __int32
	{
		Allpass,
		Bandpass,
		Highpass,
		HighShelf,
		Lowpass,
		LowShelf,
		Notch,
		Peaking
	};
}
