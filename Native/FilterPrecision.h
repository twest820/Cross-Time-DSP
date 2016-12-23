#pragma once
namespace CrossTimeDsp::Dsp
{
	public enum class FilterPrecision
	{
		Double,
		Q31,
		Q31_32x64,
		Q31_64x64,
		Q31Adaptive
	};
}