#pragma once
#include "FilterType.h"

namespace CrossTimeDsp::Dsp
{
	private class FirstOrderCoefficients
	{
	public:
		double A0;
		double A1;
		double B0;
		double B1;

		static FirstOrderCoefficients Create(FilterType type, double w0, double gainInDB);

	private:
		virtual void AdjustGain(double adjustmentInDB);
		static FirstOrderCoefficients FirstOrderCoefficients::CreateAllpass(double w0, double gainInDB);
		virtual void Normalize();
	};
}