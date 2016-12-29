#pragma once
#include "FirstOrderCoefficients.h"

namespace CrossTimeDsp::Dsp
{
	private struct BiquadCoefficients : FirstOrderCoefficients
	{
	public:
		double A2;
		double B2;

		static BiquadCoefficients Create(FilterType type, double w0, double alpha, double gainInDB);

	private:
		virtual void AdjustGain(double adjustmentInDB);
		static BiquadCoefficients BiquadCoefficients::CreateAllpass(double w0, double alpha, double gainInDB);
		static BiquadCoefficients BiquadCoefficients::CreateBandpass(double w0, double alpha, double gainInDB);
		static BiquadCoefficients BiquadCoefficients::CreateHighpass(double w0, double alpha, double gainInDB);
		static BiquadCoefficients BiquadCoefficients::CreateHighShelf(double w0, double alpha, double gainInDB);
		static BiquadCoefficients BiquadCoefficients::CreateLowShelf(double w0, double alpha, double gainInDB);
		static BiquadCoefficients BiquadCoefficients::CreateLowpass(double w0, double alpha, double gainInDB);
		static BiquadCoefficients BiquadCoefficients::CreateNotch(double w0, double alpha, double gainInDB);
		static BiquadCoefficients BiquadCoefficients::CreatePeaking(double w0, double alpha, double gainInDB);
		
		static double BiquadCoefficients::GetA(double gainInDB);
		virtual void Normalize();
	};
}
