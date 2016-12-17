using System;

namespace CrossTimeDsp.Dsp
{
    internal class Int16SampleConverter : ISampleConverter<Int32>
    {
        /// <summary>
        /// convert two little ended bytes to a Q31 integer
        /// </summary>
        /// <param name="sample">bytes to convert</param>
        /// <returns>sample in Q31 format</returns>
        public Int32 BytesToSample(byte[] sample)
        {
            // assumes a little endian processor and a little endian data stream
            // if needed, add support for big endian
            // remap as 32 bit integer and convert from Q0.15 to Q31
            return (Int32)BitConverter.ToInt16(sample, 0) << DspConstants.ShiftBetween16BitSamplesAndQ31;
        }

        /// <summary>
        /// convert a Q31 integer to two little ended bytes
        /// </summary>
        /// <param name="sample">sample in Q31 format</param>
        /// <returns>bytes corresponding to sample</returns>
        public byte[] SampleToBytes(Int32 sample)
        {
            // convert from Q31 to Q0.15, truncating least significant bits
            sample = sample >> DspConstants.ShiftBetween16BitSamplesAndQ31;
            // clip out of range values
            // if clipping is not done sign changes can occur on truncation
            if (sample > (Int32)Int16.MaxValue)
            {
                return BitConverter.GetBytes(Int16.MaxValue);
            }
            else if (sample < (Int32)Int16.MinValue)
            {
                return BitConverter.GetBytes(Int16.MinValue);
            }
            // assumes a little endian processor and a little endian data stream
            // if needed, add support for big endian
            // out of range values are truncated
            Int16 sampleAsInt16 = (Int16)sample;
            return BitConverter.GetBytes(sampleAsInt16);
        }

        public Int32 SampleToQ31(Int32 sample)
        {
            return sample;
        }
    }
}
