using System;

namespace CrossTimeDsp.Dsp
{
    internal class DoubleSampleConverter : ISampleConverter<double>
    {
        public double BytesToSample(byte[] sample)
        {
            // assumes a little endian processor and a little endian data stream
            // if needed, add support for big endian
            return BitConverter.ToDouble(sample, 0);
        }

        public double Q32ToSample(Int32 sample)
        {
            return (double)sample;
        }

        public byte[] SampleToBytes(double sample)
        {
            return BitConverter.GetBytes(sample);
        }

        public Int32 SampleToQ31(double sample)
        {
            // clip out of range values
            // if clipping is not done sign changes can occur on truncation
            if (sample > Int32.MaxValue)
            {
                return Int32.MaxValue;
            }
            else if (sample < Int32.MinValue)
            {
                return Int32.MinValue;
            }
            return (Int32)sample;
        }
    }
}
