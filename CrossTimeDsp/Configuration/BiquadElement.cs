using System;
using System.Xml.Serialization;
using CrossTimeDsp.Dsp;

namespace CrossTimeDsp.Configuration
{
    public class BiquadElement : FilterElement
    {
        [XmlAttribute(ConfigurationConstants.GainInDB)]
        public double GainInDB { get; set; }

        [XmlAttribute(ConfigurationConstants.Q)]
        public double Q { get; set; }

        private IFilter<TSample> Create<TSample>(double b0, double b1, double b2, double a0, double a1, double a2, FilterPrecision precision) where TSample : struct
        {
            switch (precision)
            {
                case FilterPrecision.Double:
                    return (IFilter<TSample>)new BiquadDouble(b0, b1, b2, a0, a1, a2);
                case FilterPrecision.Q31:
                    return (IFilter<TSample>)new BiquadQ31(b0, b1, b2, a0, a1, a2);
                case FilterPrecision.Q31_32x64:
                    return (IFilter<TSample>)new BiquadQ31_32x64(b0, b1, b2, a0, a1, a2);
                case FilterPrecision.Q31_64x64:
                    return (IFilter<TSample>)new BiquadQ31_64x64(b0, b1, b2, a0, a1, a2);
                default:
                    throw new NotSupportedException(String.Format("Unhandled filter precision {0}.", precision));
            }
        }

        internal override IFilter<TSample> Create<TSample>(double fs, EngineElement engine)
        {
            FilterPrecision precision = this.MaybeResolveAdaptiveFilterPrecision(fs, engine);
            switch (this.Type)
            {
                case FilterType.Allpass:
                    return this.CreateAllpass<TSample>(fs, precision);
                case FilterType.Bandpass:
                    return this.CreateBandpass<TSample>(fs, precision);
                case FilterType.Highpass:
                    return this.CreateHighpass<TSample>(fs, precision);
                case FilterType.HighShelf:
                    return this.CreateHighShelf<TSample>(fs, precision);
                case FilterType.Lowpass:
                    return this.CreateLowpass<TSample>(fs, precision);
                case FilterType.LowShelf:
                    return this.CreateLowShelf<TSample>(fs, precision);
                case FilterType.Notch:
                    return this.CreateNotch<TSample>(fs, precision);
                case FilterType.Peaking:
                    return this.CreatePeaking<TSample>(fs, precision);
                default:
                    throw new NotSupportedException(String.Format("Unhandled filter type {0}.", this.Type));
            }
        }

        private IFilter<TSample> CreateAllpass<TSample>(double fs, FilterPrecision precision) where TSample : struct
        {
            double w0 = this.GetW0(fs);
            double alpha = this.GetAlpha(w0);
            double b0 = 1.0 - alpha;
            double b1 = -2.0 * Math.Cos(w0);
            double b2 = 1.0 + alpha;
            double a0 = 1.0 + alpha;
            double a1 = -2.0 * Math.Cos(w0);
            double a2 = 1.0 - alpha;
            return this.Create<TSample>(b0, b1, b2, a0, a1, a2, precision);
        }

        private IFilter<TSample> CreateBandpass<TSample>(double fs, FilterPrecision precision) where TSample : struct
        {
            double w0 = this.GetW0(fs);
            double alpha = this.GetAlpha(w0);
            double b0 = alpha;
            double b1 = 0.0;
            double b2 = -alpha;
            double a0 = 1.0 + alpha;
            double a1 = -2.0 * Math.Cos(w0);
            double a2 = 1.0 - alpha;
            return this.Create<TSample>(b0, b1, b2, a0, a1, a2, precision);
        }

        private IFilter<TSample> CreateHighpass<TSample>(double fs, FilterPrecision precision) where TSample : struct
        {
            double w0 = this.GetW0(fs);
            double alpha = this.GetAlpha(w0);
            double b0 = (1.0 + Math.Cos(w0)) / 2.0;
            double b1 = -(1.0 + Math.Cos(w0));
            double b2 = (1.0 + Math.Cos(w0)) / 2.0;
            double a0 = 1.0 + alpha;
            double a1 = -2.0 * Math.Cos(w0);
            double a2 = 1.0 - alpha;
            return this.Create<TSample>(b0, b1, b2, a0, a1, a2, precision);
        }

        private IFilter<TSample> CreateHighShelf<TSample>(double fs, FilterPrecision precision) where TSample : struct
        {
            double w0 = this.GetW0(fs);
            double alpha = this.GetAlpha(w0);
            double a = this.GetA();
            double b0 = a * ((a + 1.0) + (a - 1.0) * Math.Cos(w0) + 2.0 * Math.Sqrt(a) * alpha);
            double b1 = -2.0 * a * ((a - 1.0) + (a + 1.0) * Math.Cos(w0));
            double b2 = a * ((a + 1.0) + (a - 1.0) * Math.Cos(w0) - 2.0 * Math.Sqrt(a) * alpha);
            double a0 = (a + 1.0) - (a - 1.0) * Math.Cos(w0) + 2.0 * Math.Sqrt(a) * alpha;
            double a1 = 2.0 * ((a - 1.0) - (a + 1.0) * Math.Cos(w0));
            double a2 = (a + 1.0) - (a - 1.0) * Math.Cos(w0) - 2.0 * Math.Sqrt(a) * alpha;
            return this.Create<TSample>(b0, b1, b2, a0, a1, a2, precision);
        }

        private IFilter<TSample> CreateLowShelf<TSample>(double fs, FilterPrecision precision) where TSample : struct
        {
            double w0 = this.GetW0(fs);
            double alpha = this.GetAlpha(w0);
            double a = this.GetA();
            double b0 = a * ((a + 1.0) - (a - 1.0) * Math.Cos(w0) + 2.0 * Math.Sqrt(a) * alpha);
            double b1 = 2.0 * a * ((a - 1.0) - (a + 1.0) * Math.Cos(w0));
            double b2 = a * ((a + 1.0) - (a - 1.0) * Math.Cos(w0) - 2.0 * Math.Sqrt(a) * alpha);
            double a0 = (a + 1.0) + (a - 1.0) * Math.Cos(w0) + 2.0 * Math.Sqrt(a) * alpha;
            double a1 = -2.0 * ((a - 1.0) + (a + 1.0) * Math.Cos(w0));
            double a2 = (a + 1.0) + (a - 1.0) * Math.Cos(w0) - 2.0 * Math.Sqrt(a) * alpha;
            return this.Create<TSample>(b0, b1, b2, a0, a1, a2, precision);
        }

        private IFilter<TSample> CreateLowpass<TSample>(double fs, FilterPrecision precision) where TSample : struct
        {
            double w0 = this.GetW0(fs);
            double alpha = this.GetAlpha(w0);
            double b0 = (1.0 - Math.Cos(w0)) / 2.0;
            double b1 = 1.0 - Math.Cos(w0);
            double b2 = (1.0 - Math.Cos(w0)) / 2.0;
            double a0 = 1.0 + alpha;
            double a1 = -2.0 * Math.Cos(w0);
            double a2 = 1.0 - alpha;
            return this.Create<TSample>(b0, b1, b2, a0, a1, a2, precision);
        }

        private IFilter<TSample> CreateNotch<TSample>(double fs, FilterPrecision precision) where TSample : struct
        {
            double w0 = this.GetW0(fs);
            double alpha = this.GetAlpha(w0);
            double b0 = 1.0;
            double b1 = -2.0 * Math.Cos(w0);
            double b2 = 1.0;
            double a0 = 1.0 + alpha;
            double a1 = -2.0 * Math.Cos(w0);
            double a2 = 1.0 - alpha;
            return this.Create<TSample>(b0, b1, b2, a0, a1, a2, precision);
        }

        private IFilter<TSample> CreatePeaking<TSample>(double fs, FilterPrecision precision) where TSample : struct
        {
            double w0 = this.GetW0(fs);
            double alpha = this.GetAlpha(w0);
            double a = this.GetA();
            double b0 = 1.0 + alpha * a;
            double b1 = -2.0 * Math.Cos(w0);
            double b2 = 1.0 - alpha * a;
            double a0 = 1.0 + alpha / a;
            double a1 = -2.0 * Math.Cos(w0);
            double a2 = 1.0 - alpha / a;
            return this.Create<TSample>(b0, b1, b2, a0, a1, a2, precision);
        }

        private double GetA()
        {
            return Math.Pow(10.0, this.GainInDB / 40.0);
        }

        private double GetAlpha(double w0)
        {
            return Math.Sin(w0) / (2.0 * this.Q);
        }
    }
}
