using System;
using CrossTimeDsp.Configuration;

namespace CrossTimeDsp.Dsp
{
    internal class Gain
    {
        public static IFilter<TSample> Create<TSample>(double gain, FilterPrecision precision) where TSample : struct
        {
            switch (precision)
            {
                case FilterPrecision.Double:
                    return (IFilter<TSample>)new GainDouble(gain);
                case FilterPrecision.Q31:
                case FilterPrecision.Q31_32x64:
                case FilterPrecision.Q31Adaptive:
                    return (IFilter<TSample>)new GainQ31(gain);
                case FilterPrecision.Q31_64x64:
                    return (IFilter<TSample>)new GainQ31_64x64(gain);
                default:
                    throw new NotSupportedException(String.Format("Unhandled filter precision {0}.", precision));
            }
        }
    }
}
