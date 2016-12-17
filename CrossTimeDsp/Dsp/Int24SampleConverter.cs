using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace CrossTimeDsp.Dsp
{
    public class Int24SampleConverter : ISampleConverter<Int32>
    {
        private static readonly Int32 Int24MaxValue = 16777215;
        private static readonly Int32 Int24MinValue = -16777216;

        /// <summary>
        /// convert three little ended bytes to a Q31 integer
        /// </summary>
        /// <param name="sample">bytes to convert</param>
        /// <returns>sample in Q31 format</returns>
        public Int32 BytesToSample(byte[] sample)
        {
            // if needed, add support for big endian
            // 24 bit value is taken directly to Q31
            byte[] rawDataAsInt24 = new byte[] { sample[0], sample[1], sample[2], 0 };
            if (sample[2] >= 0x80)
            {
                rawDataAsInt24[3] = 0xff;
            }
            // remap as 32 bit integer and convert from Q0.23 to Q31
            return BitConverter.ToInt32(rawDataAsInt24, 0) << DspConstants.ShiftBetween24BitSamplesAndQ31;
        }

        /// <summary>
        /// convert a Q31 integer to three little ended bytes
        /// </summary>
        /// <param name="sample">sample in Q31 format</param>
        /// <returns>bytes corresponding to sample</returns>
        public byte[] SampleToBytes(Int32 sample)
        {
            // convert from Q31 to Q0.23, truncating least significant bits
            sample = sample >> DspConstants.ShiftBetween24BitSamplesAndQ31;
            // clip out of range values
            // if clipping is not done sign changes can occur on truncation
            if (sample > Int24SampleConverter.Int24MaxValue)
            {
                sample = Int24SampleConverter.Int24MaxValue;
            }
            else if (sample < Int24SampleConverter.Int24MinValue)
            {
                sample = Int24SampleConverter.Int24MinValue;
            }
            // if needed, add support for big endian
            // out of range values are truncated
            byte[] sampleAsInt32 = BitConverter.GetBytes(sample);
            byte[] sampleAsInt24 = new byte[] { sampleAsInt32[0], sampleAsInt32[1], sampleAsInt32[2] };
            return sampleAsInt24;
        }

        public Int32 SampleToQ31(Int32 sample)
        {
            return sample;
        }
    }
}
