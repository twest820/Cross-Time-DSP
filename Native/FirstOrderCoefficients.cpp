#include "stdafx.h"
#include <cmath>
#include "FirstOrderCoefficients.h"

using namespace System;

namespace CrossTimeDsp::Dsp
{
	FirstOrderCoefficients FirstOrderCoefficients::Create(FilterType type, double w0, double gainInDB)
	{
		switch (type)
		{
		case FilterType::Allpass:
			return FirstOrderCoefficients::CreateAllpass(w0, gainInDB);
		default:
			throw gcnew NotSupportedException(String::Format("Unhandled filter type {0}.", type));
		}
	}

	void FirstOrderCoefficients::AdjustGain(double adjustmentInDB)
	{
		if (adjustmentInDB == 0.0)
		{
			return;
		}

		double adjustment = std::pow(10.0, adjustmentInDB / 20.0);
		this->B0 *= adjustment;
		this->B1 *= adjustment;
	}

	FirstOrderCoefficients FirstOrderCoefficients::CreateAllpass(double w0, double gainInDB)
	{
		double c = (std::tan(w0 / 2.0) - 1.0) / (std::tan(w0 / 2.0) + 1.0);
		FirstOrderCoefficients coefficients;
		coefficients.B0 = c;
		coefficients.B1 = 1.0;
		coefficients.A0 = 1.0;
		coefficients.A1 = c;
		coefficients.Normalize();
		coefficients.AdjustGain(gainInDB);
		return coefficients;
	}

	void FirstOrderCoefficients::Normalize()
	{
		this->A1 /= this->A0;
		this->B0 /= this->A0;
		this->B1 /= this->A0;
		this->A0 = 1.0;
	}
}