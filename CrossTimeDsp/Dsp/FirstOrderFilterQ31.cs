using System;

namespace CrossTimeDsp.Dsp
{
    /// <summary>
    /// direct form 1 implementation of a first order filter using a Q63 state variable for increased accuracy
    /// </summary>
    internal class FirstOrderFilterQ31 : IFilter<Int32>
    {
        private Q31 a1;
        private Q31 b0;
        private Q31 b1;
        private int fractionalBitsInCoefficients;
        private Int32 x1;
        private Int32 y1;

        public FirstOrderFilterQ31(double b0, double b1, double a0, double a1)
        {
            a1 /= a0;
            b0 /= a0;
            b1 /= a0;
            this.fractionalBitsInCoefficients = Q31.GetOptimalNumberOfFractionalBits(a1, b0, b1);
            this.a1 = new Q31(a1, this.fractionalBitsInCoefficients);
            this.b0 = new Q31(b0, this.fractionalBitsInCoefficients);
            this.b1 = new Q31(b1, this.fractionalBitsInCoefficients);
            this.x1 = 0;
            this.y1 = 0;
        }

        private FirstOrderFilterQ31(FirstOrderFilterQ31 other)
        {
            this.a1 = other.a1;
            this.b0 = other.b0;
            this.b1 = other.b1;
            this.fractionalBitsInCoefficients = other.fractionalBitsInCoefficients;
            this.x1 = other.x1;
            this.y1 = other.y1;
        }

        public IFilter<Int32> Clone()
        {
            return new FirstOrderFilterQ31(this);
        }

        public Int32 Filter(Int32 sample)
        {
            Int64 accumulator = this.b0 * sample + this.b1 * this.x1 - this.a1 * this.y1;
            this.x1 = sample;
            this.y1 = (Int32)(accumulator >> this.fractionalBitsInCoefficients);
            return this.y1;
        }
    }
}