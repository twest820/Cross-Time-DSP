﻿using System;

namespace CrossTimeDsp.Dsp
{
    /// <summary>
    /// typical fixed point direct form 1 biquad implementation
    /// </summary>
    internal class BiquadQ31 : IFilter<Int32>
    {
        private Q31 a1;
        private Q31 a2;
        private Q31 b0;
        private Q31 b1;
        private Q31 b2;
        private int fractionalBitsInCoefficients;
        private Int32 x1;
        private Int32 x2;
        private Int32 y1;
        private Int32 y2;

        public BiquadQ31(double b0, double b1, double b2, double a0, double a1, double a2)
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
            this.x1 = 0;
            this.x2 = 0;
            this.y1 = 0;
            this.y2 = 0;
        }

        private BiquadQ31(BiquadQ31 other)
        {
            this.a1 = other.a1;
            this.a2 = other.a2;
            this.b0 = other.b0;
            this.b1 = other.b1;
            this.b2 = other.b2;
            this.fractionalBitsInCoefficients = other.fractionalBitsInCoefficients;
            this.x1 = other.x1;
            this.x2 = other.x2;
            this.y1 = other.y1;
            this.y2 = other.y2;
        }

        public IFilter<Int32> Clone()
        {
            return new BiquadQ31(this);
        }

        /// <summary>
        /// Process a single sample.  The sample must be in [ 0.25, 0.25 ) to guarantee the accumulator does not overflow and wrap around.
        /// </summary>
        public Int32 Filter(Int32 sample)
        {
            Int64 accumulator = this.b0 * sample + this.b1 * this.x1 + this.b2 * this.x2 - this.a1 * this.y1 - this.a2 * this.y2;
            this.x2 = this.x1;
            this.x1 = sample;
            this.y2 = this.y1;
            this.y1 = (Int32)(accumulator >> this.fractionalBitsInCoefficients);
            return this.y1;
        }
    }
}