using CrossTimeDsp.Dsp;
using System;
using System.Xml.Serialization;

namespace CrossTimeDsp.Configuration
{
    /// <summary>
    /// Specification of a first order filter.
    /// </summary>
    public class FilterElement
    {
        [XmlAttribute(ConfigurationConstants.F0)]
        public double F0 { get; set; }

        [XmlAttribute(ConfigurationConstants.TimeDirection)]
        public TimeDirection TimeDirection { get; set; }

        [XmlAttribute(ConfigurationConstants.Type)]
        public FilterType Type { get; set; }

        private IFilter<TSample> Create<TSample>(double b0, double b1, double a0, double a1, FilterPrecision precision, int channels) where TSample : struct
        {
            switch (precision)
            {
                case FilterPrecision.Double:
                    return (IFilter<TSample>)new FirstOrderFilterDouble(b0, b1, a0, a1, channels);
                case FilterPrecision.Q31:
                    return (IFilter<TSample>)new FirstOrderFilterQ31(b0, b1, a0, a1, channels);
                case FilterPrecision.Q31_32x64:
                    return (IFilter<TSample>)new FirstOrderFilterQ31_32x64(b0, b1, a0, a1, channels);
                case FilterPrecision.Q31_64x64:
                    return (IFilter<TSample>)new FirstOrderFilterQ31_32x64(b0, b1, a0, a1, channels);
                default:
                    throw new NotSupportedException(String.Format("Unhandled filter precision {0}.", precision));
            }
        }

        internal virtual IFilter<TSample> Create<TSample>(double fs, EngineElement engine, int channels) where TSample : struct
        {
            FilterPrecision precision = this.MaybeResolveAdaptiveFilterPrecision(fs, engine);
            switch (this.Type)
            {
                case FilterType.Allpass:
                    return this.CreateAllpassDouble<TSample>(fs, precision, channels);
                default:
                    throw new NotSupportedException(String.Format("Unhandled filter type {0}.", this.Type));
            }
        }

        private IFilter<TSample> CreateAllpassDouble<TSample>(double fs, FilterPrecision precision, int channels) where TSample : struct
        {
            double w0 = this.GetW0(fs);
            double c = (Math.Tan(w0 / 2.0) - 1.0) / (Math.Tan(w0 / 2.0) + 1.0);
            double b0 = c;
            double b1 = 1.0;
            double a0 = 1.0;
            double a1 = c;
            return this.Create<TSample>(b0, b1, a0, a1, precision, channels);
        }

        protected double GetW0(double fs)
        {
            return 2.0 * Math.PI * this.F0 / fs;
        }

        protected FilterPrecision MaybeResolveAdaptiveFilterPrecision(double fs, EngineElement engine)
        {
            if (engine.Precision != FilterPrecision.Q31Adaptive)
            {
                return engine.Precision;
            }
            double normalizedCenterFrequency = this.F0 / fs;
            if (normalizedCenterFrequency > engine.Q31Adaptive.Q31_32x64_Threshold)
            {
                return FilterPrecision.Q31;
            }
            else if (normalizedCenterFrequency > engine.Q31Adaptive.Q31_64x64_Threshold)
            {
                return FilterPrecision.Q31_32x64;
            }
            else
            {
                return FilterPrecision.Q31_64x64;
            }
        }
    }
}
