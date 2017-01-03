#include "stdafx.h"
#include <cmath>
#include <stdexcept>
#include <string>
#include "BiquadCoefficients.h"

namespace CrossTimeDsp::Dsp
{
	BiquadCoefficients BiquadCoefficients::Create(FilterTypeNative type, double w0, double alpha, double gainInDB)
	{
		switch (type)
		{
		case FilterTypeNative::Allpass:
			return BiquadCoefficients::CreateAllpass(w0, alpha, gainInDB);
		case FilterTypeNative::Bandpass:
			return BiquadCoefficients::CreateBandpass(w0, alpha, gainInDB);
		case FilterTypeNative::Highpass:
			return BiquadCoefficients::CreateHighpass(w0, alpha, gainInDB);
		case FilterTypeNative::HighShelf:
			return BiquadCoefficients::CreateHighShelf(w0, alpha, gainInDB);
		case FilterTypeNative::Lowpass:
			return BiquadCoefficients::CreateLowpass(w0, alpha, gainInDB);
		case FilterTypeNative::LowShelf:
			return BiquadCoefficients::CreateLowShelf(w0, alpha, gainInDB);
		case FilterTypeNative::Notch:
			return BiquadCoefficients::CreateNotch(w0, alpha, gainInDB);
		case FilterTypeNative::Peaking:
			return BiquadCoefficients::CreatePeaking(w0, alpha, gainInDB);
		default:
			throw std::invalid_argument("Unhandled filter type " + std::to_string((__int32)type) + ".");
		}
	}

	void BiquadCoefficients::AdjustGain(double adjustmentInDB)
	{
		if (adjustmentInDB == 0.0)
		{
			return;
		}

		double adjustment = std::pow(10.0, adjustmentInDB / 20.0);
		this->B0 *= adjustment;
		this->B1 *= adjustment;
		this->B2 *= adjustment;
	}

	BiquadCoefficients BiquadCoefficients::CreateAllpass(double w0, double alpha, double gainInDB)
	{
		BiquadCoefficients coefficients;
		coefficients.B0 = 1.0 - alpha;
		coefficients.B1 = -2.0 * std::cos(w0);
		coefficients.B2 = 1.0 + alpha;
		coefficients.A0 = 1.0 + alpha;
		coefficients.A1 = -2.0 * std::cos(w0);
		coefficients.A2 = 1.0 - alpha;
		coefficients.Normalize();
		coefficients.AdjustGain(gainInDB);
		return coefficients;
	}

	BiquadCoefficients BiquadCoefficients::CreateBandpass(double w0, double alpha, double gainInDB)
	{
		BiquadCoefficients coefficients;
		coefficients.B0 = alpha;
		coefficients.B1 = 0.0;
		coefficients.B2 = -alpha;
		coefficients.A0 = 1.0 + alpha;
		coefficients.A1 = -2.0 * std::cos(w0);
		coefficients.A2 = 1.0 - alpha;
		coefficients.Normalize();
		return coefficients;
	}

	BiquadCoefficients BiquadCoefficients::CreateHighpass(double w0, double alpha, double gainInDB)
	{
		BiquadCoefficients coefficients;
		coefficients.B0 = (1.0 + std::cos(w0)) / 2.0;
		coefficients.B1 = -(1.0 + std::cos(w0));
		coefficients.B2 = (1.0 + std::cos(w0)) / 2.0;
		coefficients.A0 = 1.0 + alpha;
		coefficients.A1 = -2.0 * std::cos(w0);
		coefficients.A2 = 1.0 - alpha;
		coefficients.Normalize();
		coefficients.AdjustGain(gainInDB);
		return coefficients;
	}

	BiquadCoefficients BiquadCoefficients::CreateHighShelf(double w0, double alpha, double gainInDB)
	{
		double a = BiquadCoefficients::GetA(gainInDB);
		BiquadCoefficients coefficients;
		coefficients.B0 = a * ((a + 1.0) + (a - 1.0) * std::cos(w0) + 2.0 * std::sqrt(a) * alpha);
		coefficients.B1 = -2.0 * a * ((a - 1.0) + (a + 1.0) * std::cos(w0));
		coefficients.B2 = a * ((a + 1.0) + (a - 1.0) * std::cos(w0) - 2.0 * std::sqrt(a) * alpha);
		coefficients.A0 = (a + 1.0) - (a - 1.0) * std::cos(w0) + 2.0 * std::sqrt(a) * alpha;
		coefficients.A1 = 2.0 * ((a - 1.0) - (a + 1.0) * std::cos(w0));
		coefficients.A2 = (a + 1.0) - (a - 1.0) * std::cos(w0) - 2.0 * std::sqrt(a) * alpha;
		coefficients.Normalize();
		return coefficients;
	}

	BiquadCoefficients BiquadCoefficients::CreateLowShelf(double w0, double alpha, double gainInDB)
	{
		double a = BiquadCoefficients::GetA(gainInDB);
		BiquadCoefficients coefficients;
		coefficients.B0 = a * ((a + 1.0) - (a - 1.0) * std::cos(w0) + 2.0 * std::sqrt(a) * alpha);
		coefficients.B1 = 2.0 * a * ((a - 1.0) - (a + 1.0) * std::cos(w0));
		coefficients.B2 = a * ((a + 1.0) - (a - 1.0) * std::cos(w0) - 2.0 * std::sqrt(a) * alpha);
		coefficients.A0 = (a + 1.0) + (a - 1.0) * std::cos(w0) + 2.0 * std::sqrt(a) * alpha;
		coefficients.A1 = -2.0 * ((a - 1.0) + (a + 1.0) * std::cos(w0));
		coefficients.A2 = (a + 1.0) + (a - 1.0) * std::cos(w0) - 2.0 * std::sqrt(a) * alpha;
		coefficients.Normalize();
		return coefficients;
	}

	BiquadCoefficients BiquadCoefficients::CreateLowpass(double w0, double alpha, double gainInDB)
	{
		BiquadCoefficients coefficients;
		coefficients.B0 = (1.0 - std::cos(w0)) / 2.0;
		coefficients.B1 = 1.0 - std::cos(w0);
		coefficients.B2 = (1.0 - std::cos(w0)) / 2.0;
		coefficients.A0 = 1.0 + alpha;
		coefficients.A1 = -2.0 * std::cos(w0);
		coefficients.A2 = 1.0 - alpha;
		coefficients.Normalize();
		coefficients.AdjustGain(gainInDB);
		return coefficients;
	}

	BiquadCoefficients BiquadCoefficients::CreateNotch(double w0, double alpha, double gainInDB)
	{
		BiquadCoefficients coefficients;
		coefficients.B0 = 1.0;
		coefficients.B1 = -2.0 * std::cos(w0);
		coefficients.B2 = 1.0;
		coefficients.A0 = 1.0 + alpha;
		coefficients.A1 = -2.0 * std::cos(w0);
		coefficients.A2 = 1.0 - alpha;
		coefficients.Normalize();
		coefficients.AdjustGain(gainInDB);
		return coefficients;
	}

	BiquadCoefficients BiquadCoefficients::CreatePeaking(double w0, double alpha, double gainInDB)
	{
		double a = BiquadCoefficients::GetA(gainInDB);
		BiquadCoefficients coefficients;
		coefficients.B0 = 1.0 + alpha * a;
		coefficients.B1 = -2.0 * std::cos(w0);
		coefficients.B2 = 1.0 - alpha * a;
		coefficients.A0 = 1.0 + alpha / a;
		coefficients.A1 = -2.0 * std::cos(w0);
		coefficients.A2 = 1.0 - alpha / a;
		coefficients.Normalize();
		return coefficients;
	}

	double BiquadCoefficients::GetA(double gainInDB)
	{
		return std::pow(10.0, gainInDB / 40.0);
	}

	void BiquadCoefficients::Normalize()
	{
		this->A1 /= this->A0;
		this->A2 /= this->A0;
		this->B0 /= this->A0;
		this->B1 /= this->A0;
		this->B2 /= this->A0;
		this->A0 = 1.0;
	}
}