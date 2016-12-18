using System;

namespace CrossTimeDsp.Dsp
{
    /// <summary>
    /// direct form 1 implementation of a first order filter using a Q63 state variable for increased accuracy
    /// </summary>
    internal class FirstOrderFilterQ31_32x64 : IFilter<int>
    {
        private Q31 a1;
        private Q31 b0;
        private Q31 b1;
        private int channels;
        private int postShift;
        private int[] x1;
        private long[] y1;

        public FirstOrderFilterQ31_32x64(double b0, double b1, double a0, double a1, int channels)
        {
            a1 /= a0;
            b0 /= a0;
            b1 /= a0;
            // see remarks in BiquadQ31_32x64..ctor()
            this.postShift = Q31.GetQ31_32x64_PostShift(a1, b0, b1);
            double coefficientScaling = Math.Pow(2.0, -this.postShift);
            this.a1 = new Q31(coefficientScaling * a1, Q31.MaximumFractionalBits);
            this.b0 = new Q31(coefficientScaling * b0, Q31.MaximumFractionalBits);
            this.b1 = new Q31(coefficientScaling * b1, Q31.MaximumFractionalBits);

            this.channels = channels;
            this.x1 = new int[channels];
            this.y1 = new long[channels];
        }

        public void Filter(int[] block, int offset)
        {
            int maxSample = offset + Constant.FilterBlockSizeInInts;
            for (int sample = offset; sample < maxSample; ++sample)
            {
                int channel = sample % this.channels;
                int value = block[sample];
                long accumulator = this.b0 * value + this.b1 * this.x1[channel] - this.a1 * this.y1[channel];
                this.x1[channel] = value;
                this.y1[channel] = accumulator << this.postShift + 1;
                block[sample] = (int)(accumulator >> Q31.MaximumFractionalBits - this.postShift);
            }
        }

        public void FilterReverse(int[] block, int offset)
        {
            int maxSample = offset + Constant.FilterBlockSizeInInts;
            for (int sample = maxSample - 1; sample >= offset; --sample)
            {
                int channel = sample % this.channels;
                int value = block[sample];
                long accumulator = this.b0 * value + this.b1 * this.x1[channel] - this.a1 * this.y1[channel];
                this.x1[channel] = value;
                this.y1[channel] = accumulator << this.postShift + 1;
                block[sample] = (int)(accumulator >> Q31.MaximumFractionalBits - this.postShift);
            }
        }
    }
}