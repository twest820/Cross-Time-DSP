using System;
using System.Runtime.InteropServices;

namespace CrossTimeDsp.Dsp
{
    [StructLayout(LayoutKind.Explicit, Pack = 4)]
    internal class SampleBlock
    {
        [FieldOffset(0)]
        private int bytesInUse;

        [FieldOffset(4)]
        private SampleType sampleType;

        [FieldOffset(8)]
        private byte[] byteBuffer;
        [FieldOffset(8)]
        private double[] doubleBuffer;
        [FieldOffset(8)]
        private int[] intBuffer;
        [FieldOffset(8)]
        private short[] shortBuffer;

        public SampleBlock(int requestedSizeInBytes, SampleType sampleType)
        {
            // round size up to filter granularity
            int filterBlocks = requestedSizeInBytes / Constant.FilterBlockSizeInBytes + 1;
            int bytesToAllocate = filterBlocks * Constant.FilterBlockSizeInBytes;

            this.byteBuffer = new byte[bytesToAllocate];
            this.bytesInUse = 0;
            this.sampleType = sampleType;
        }

        public byte[] ByteBuffer
        {
            get { return this.byteBuffer; }
        }

        public int ByteBufferCount
        {
            get { return this.bytesInUse; }
        }

        public int BytesInUse
        {
            get { return this.bytesInUse; }
            set { this.bytesInUse = value; }
        }

        public double[] DoubleBuffer
        {
            get { return this.doubleBuffer; }
        }

        public int DoubleBufferCount
        {
            get { return this.bytesInUse / 8; }
        }

        public int[] IntBuffer
        {
            get { return this.intBuffer; }
        }

        public int IntBufferCount
        {
            get { return this.bytesInUse / 4; }
        }

        public int MaximumSizeInBytes
        {
            get { return this.byteBuffer.Length; }
        }

        public int SamplesInUse
        {
            get { return this.BytesInUse / this.SampleType.BytesPerSample(); }
        }

        public SampleType SampleType
        {
            get { return this.sampleType; }
            set { this.sampleType = value; }
        }

        public short[] ShortBuffer
        {
            get { return this.shortBuffer; }
        }

        public int ShortBufferCount
        {
            get { return this.bytesInUse / 2; }
        }

        private void ConvertDoubleToQ15(SampleBlock convertedBlock)
        {
            for (int sample = 0; sample < this.DoubleBufferCount; ++sample)
            {
                int value = (int)this.DoubleBuffer[sample] >> Constant.Dsp.ShiftBetween16BitSamplesAndQ31;
                if (value > Int16.MaxValue)
                {
                    convertedBlock.ShortBuffer[sample] = Int16.MaxValue;
                }
                else if (value < Int16.MinValue)
                {
                    convertedBlock.ShortBuffer[sample] = Int16.MinValue;
                }
                else
                {
                    convertedBlock.ShortBuffer[sample] = (short)value;
                }
            }
        }

        private void ConvertDoubleToQ23(SampleBlock convertedBlock)
        {
            for (int sample = 0; sample < this.DoubleBufferCount; ++sample)
            {
                this.SetInt32AsInt24((int)this.DoubleBuffer[sample] >> Constant.Dsp.ShiftBetween24BitSamplesAndQ31, convertedBlock, sample);
            }
        }

        private void ConvertDoubleToQ31(SampleBlock convertedBlock)
        {
            for (int sample = 0; sample < this.DoubleBufferCount; ++sample)
            {
                double value = this.DoubleBuffer[sample];
                if (value > Int32.MaxValue)
                {
                    convertedBlock.IntBuffer[sample] = Int32.MaxValue;
                }
                else if (value < Int32.MinValue)
                {
                    convertedBlock.IntBuffer[sample] = Int32.MinValue;
                }
                else
                {
                    convertedBlock.IntBuffer[sample] = (int)value;
                }
            }
        }

        private void ConvertQ15ToDouble(SampleBlock convertedBlock)
        {
            for (int sample = 0; sample < this.ShortBufferCount; ++sample)
            {
                convertedBlock.DoubleBuffer[sample] = (int)this.ShortBuffer[sample] << Constant.Dsp.ShiftBetween16BitSamplesAndQ31;
            }
        }

        private void ConvertQ15ToQ31(SampleBlock convertedBlock)
        {
            for (int sample = 0; sample < this.ShortBufferCount; ++sample)
            {
                convertedBlock.IntBuffer[sample] = (int)this.ShortBuffer[sample] << Constant.Dsp.ShiftBetween16BitSamplesAndQ31;
            }
        }

        private void ConvertQ23ToDouble(SampleBlock convertedBlock)
        {
            for (int sample = 0; sample < this.DoubleBufferCount; ++sample)
            {
                convertedBlock.DoubleBuffer[sample] = (double)(this.GetInt24AsInt32(sample) << Constant.Dsp.ShiftBetween24BitSamplesAndQ31);
            }
        }

        private void ConvertQ23ToQ31(SampleBlock convertedBlock)
        {
            for (int sample = 0; sample < this.IntBufferCount; ++sample)
            {
                convertedBlock.IntBuffer[sample] = this.GetInt24AsInt32(sample) << Constant.Dsp.ShiftBetween24BitSamplesAndQ31;
            }
        }

        private void ConvertQ31ToDouble(SampleBlock convertedBlock)
        {
            for (int sample = 0; sample < this.IntBufferCount; ++sample)
            {
                convertedBlock.DoubleBuffer[sample] = this.IntBuffer[sample];
            }
        }

        private void ConvertQ31ToQ15(SampleBlock convertedBlock)
        {
            for (int sample = 0; sample < this.IntBufferCount; ++sample)
            {
                int value = this.IntBuffer[sample] >> Constant.Dsp.ShiftBetween16BitSamplesAndQ31;
                if (value > Int16.MaxValue)
                {
                    convertedBlock.ShortBuffer[sample] = Int16.MaxValue;
                }
                else if (value < Int16.MinValue)
                {
                    convertedBlock.ShortBuffer[sample] = Int16.MinValue;
                }
                else
                {
                    convertedBlock.ShortBuffer[sample] = (short)value;
                }
            }
        }

        private void ConvertQ31ToQ23(SampleBlock convertedBlock)
        {
            for (int sample = 0; sample < this.IntBufferCount; ++sample)
            {
                this.SetInt32AsInt24(this.IntBuffer[sample] >> Constant.Dsp.ShiftBetween24BitSamplesAndQ31, convertedBlock, sample);
            }
        }

        public SampleBlock ConvertTo(SampleType sampleType)
        {
            if (this.sampleType == sampleType)
            {
                return this;
            }

            int convertedSizeInBytes = sampleType.BytesPerSample() * this.SamplesInUse;
            SampleBlock convertedBlock = new SampleBlock(convertedSizeInBytes, sampleType);
            switch (this.sampleType)
            {
                case SampleType.Double:
                    switch (sampleType)
                    {
                        case SampleType.Int16:
                            this.ConvertDoubleToQ15(convertedBlock);
                            break;
                        case SampleType.Int24:
                            this.ConvertDoubleToQ23(convertedBlock);
                            break;
                        case SampleType.Int32:
                            this.ConvertDoubleToQ31(convertedBlock);
                            break;
                        default:
                            throw new NotSupportedException(String.Format("Conversion from {0} to {1} is not supported.", this.sampleType, sampleType));
                    }
                    break;
                case SampleType.Int16:
                    switch (sampleType)
                    {
                        case SampleType.Double:
                            this.ConvertQ15ToDouble(convertedBlock);
                            break;
                        case SampleType.Int32:
                            this.ConvertQ15ToQ31(convertedBlock);
                            break;
                        default:
                            throw new NotSupportedException(String.Format("Conversion from {0} to {1} is not supported.", this.sampleType, sampleType));
                    }
                    break;
                case SampleType.Int24:
                    switch (sampleType)
                    {
                        case SampleType.Double:
                            this.ConvertQ23ToDouble(convertedBlock);
                            break;
                        case SampleType.Int32:
                            this.ConvertQ23ToQ31(convertedBlock);
                            break;
                        default:
                            throw new NotSupportedException(String.Format("Conversion from {0} to {1} is not supported.", this.sampleType, sampleType));
                    }
                    break;
                case SampleType.Int32:
                    switch (sampleType)
                    {
                        case SampleType.Double:
                            this.ConvertQ31ToDouble(convertedBlock);
                            break;
                        case SampleType.Int16:
                            this.ConvertQ31ToQ15(convertedBlock);
                            break;
                        case SampleType.Int24:
                            this.ConvertQ31ToQ23(convertedBlock);
                            break;
                        default:
                            throw new NotSupportedException(String.Format("Conversion from {0} to {1} is not supported.", this.sampleType, sampleType));
                    }
                    break;
                default:
                    throw new NotSupportedException(String.Format("Conversion from sample type {0} is not supported.", this.sampleType));
            }

            convertedBlock.BytesInUse = convertedSizeInBytes;
            return convertedBlock;
        }

        public int GetInt24AsInt32(int sample)
        {
            int blockIndex = 3 * sample;
            byte[] q24 = new byte[] { this.ByteBuffer[blockIndex], this.ByteBuffer[blockIndex + 1], this.ByteBuffer[blockIndex + 2], 0 };
            if (q24[2] >= 0x80)
            {
                q24[3] = 0xff;
            }

            return BitConverter.ToInt32(q24, 0);
        }

        internal void SetInt32AsInt24(int value, SampleBlock convertedBlock, int sample)
        {
            int blockIndex = 3 * sample;
            if (value > Constant.Int24.MaxValue)
            {
                convertedBlock.ByteBuffer[blockIndex] = Constant.Int24.MaxValueBytes[0];
                convertedBlock.ByteBuffer[blockIndex + 1] = Constant.Int24.MaxValueBytes[1];
                convertedBlock.ByteBuffer[blockIndex + 2] = Constant.Int24.MaxValueBytes[2];
            }
            else if (value < Constant.Int24.MinValue)
            {
                convertedBlock.ByteBuffer[blockIndex] = Constant.Int24.MinValueBytes[0];
                convertedBlock.ByteBuffer[blockIndex + 1] = Constant.Int24.MinValueBytes[1];
                convertedBlock.ByteBuffer[blockIndex + 2] = Constant.Int24.MinValueBytes[2];
            }
            else
            {
                byte[] q32Bytes = BitConverter.GetBytes(value);
                convertedBlock.ByteBuffer[blockIndex] = q32Bytes[0];
                convertedBlock.ByteBuffer[blockIndex + 1] = q32Bytes[1];
                convertedBlock.ByteBuffer[blockIndex + 2] = q32Bytes[2];
            }
        }
    }
}
