using System;

namespace CrossTimeDsp.Dsp
{
    internal class Int32SampleConverter : ISampleConverter<Int32>
    {
        /// <summary>
        /// convert four little ended bytes to a Q31 integer
        /// </summary>
        /// <param name="bytes">bytes to convert</param>
        /// <returns>sample in Q31 format</returns>
        public Int32 BytesToSample(byte[] bytes)
        {
            return BitConverter.ToInt32(bytes, 0);
        }

        /// <summary>
        /// convert a Q31 integer to four little ended bytes
        /// </summary>
        /// <param name="sample">sample in Q31 format</param>
        /// <returns>bytes corresponding to sample</returns>
        public byte[] SampleToBytes(Int32 sample)
        {
            return BitConverter.GetBytes(sample);
        }

        public Int32 SampleToQ31(Int32 sample)
        {
            return sample;
        }
    }
}