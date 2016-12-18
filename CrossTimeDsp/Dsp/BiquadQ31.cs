namespace CrossTimeDsp.Dsp
{
    /// <summary>
    /// typical fixed point direct form 1 biquad implementation
    /// </summary>
    internal class BiquadQ31 : IFilter<int>
    {
        private Q31 a1;
        private Q31 a2;
        private Q31 b0;
        private Q31 b1;
        private Q31 b2;
        private int channels;
        private int fractionalBitsInCoefficients;
        private int[] x1;
        private int[] x2;
        private int[] y1;
        private int[] y2;

        public BiquadQ31(double b0, double b1, double b2, double a0, double a1, double a2, int channels)
        {
            a1 /= a0;
            a2 /= a0;
            b0 /= a0;
            b1 /= a0;
            b2 /= a0;
            this.fractionalBitsInCoefficients = Q31.GetOptimalNumberOfFractionalBits(a1, a2, b0, b1, b2);
            this.a1 = new Q31(a1, this.fractionalBitsInCoefficients);
            this.a2 = new Q31(a2, this.fractionalBitsInCoefficients);
            this.b0 = new Q31(b0, this.fractionalBitsInCoefficients);
            this.b1 = new Q31(b1, this.fractionalBitsInCoefficients);
            this.b2 = new Q31(b2, this.fractionalBitsInCoefficients);

            this.channels = channels;
            this.x1 = new int[channels];
            this.x2 = new int[channels];
            this.y1 = new int[channels];
            this.y2 = new int[channels];
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
                this.y1[channel] = (int)(accumulator >> this.fractionalBitsInCoefficients);
                block[sample] = this.y1[channel];
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
                this.y1[channel] = (int)(accumulator >> this.fractionalBitsInCoefficients);
                block[sample] = this.y1[channel];
            }
        }
    }
}