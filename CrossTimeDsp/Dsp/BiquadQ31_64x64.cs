using System;

namespace CrossTimeDsp.Dsp
{
    /// <summary>
    /// direct form 1 biquad implementation using Q63 coefficients and Q63 feedback state variables
    /// </summary>
    internal class BiquadQ31_64x64 : IFilter<int>
    {
        private Q63 a1;
        private Q63 a2;
        private Q63 b0;
        private Q63 b1;
        private Q63 b2;
        private int channels;
        private int postShift;
        private int[] x1;
        private int[] x2;
        private long[] y1;
        private long[] y2;

        public BiquadQ31_64x64(double b0, double b1, double b2, double a0, double a1, double a2, int channels)
        {
            a1 /= a0;
            a2 /= a0;
            b0 /= a0;
            b1 /= a0;
            b2 /= a0;
            // scale coefficients into Q0.31 so that state variables can be stored in Q0.63 for maximum precision
            // the post shift is used to undo the scaling at output
            this.postShift = Q63.GetQ31_64x64_PostShift(a1, a2, b0, b1, b2);
            double coefficientScaling = Math.Pow(2.0, -this.postShift);
            this.a1 = new Q63(coefficientScaling * a1, Q63.MaximumFractionalBits);
            this.a2 = new Q63(coefficientScaling * a2, Q63.MaximumFractionalBits);
            this.b0 = new Q63(coefficientScaling * b0, Q63.MaximumFractionalBits);
            this.b1 = new Q63(coefficientScaling * b1, Q63.MaximumFractionalBits);
            this.b2 = new Q63(coefficientScaling * b2, Q63.MaximumFractionalBits);

            this.channels = channels;
            this.x1 = new int[channels];
            this.x2 = new int[channels];
            this.y1 = new long[channels];
            this.y2 = new long[channels];
        }

        /// <summary>
        /// Process a single sample.  The sample must be in [ 0.25, 0.25 ) to guarantee the accumulator does not overflow and wrap around.
        /// </summary>
        public void Filter(int[] block, int offset)
        {
            int maxSample = offset + Constant.FilterBlockSizeInInts;
            for (int sample = offset; sample < maxSample; ++sample)
            {
                int channel = sample % this.channels;
                int value = block[sample];
                long accumulator = this.b0 * value + this.b1 * this.x1[channel] + this.b2 * this.x2[channel] - this.a1 * this.y1[channel] - this.a2 * this.y2[channel];
                this.x2[channel] = this.x1[channel];
                this.x1[channel] = value;
                this.y2[channel] = this.y1[channel];
                this.y1[channel] = accumulator << this.postShift + 1;
                // output conversion is from a Q63 accumulator to Q31, so reference Q31.MaximumFractionalBits for the shift
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
                long accumulator = this.b0 * value + this.b1 * this.x1[channel] + this.b2 * this.x2[channel] - this.a1 * this.y1[channel] - this.a2 * this.y2[channel];
                this.x2[channel] = this.x1[channel];
                this.x1[channel] = value;
                this.y2[channel] = this.y1[channel];
                this.y1[channel] = accumulator << this.postShift + 1;
                block[sample] = (int)(accumulator >> Q31.MaximumFractionalBits - this.postShift);
            }
        }
    }
}
