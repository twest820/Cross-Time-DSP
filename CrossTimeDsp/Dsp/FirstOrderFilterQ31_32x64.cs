using System;

namespace CrossTimeDsp.Dsp
{
    /// <summary>
    /// direct form 1 implementation of a first order filter using a Q63 state variable for increased accuracy
    /// </summary>
    internal class FirstOrderFilterQ31_32x64 : IFilter<Int32>
    {
        private Q31 a1;
        private Q31 b0;
        private Q31 b1;
        private int postShift;
        private Int32 x1;
        private Int64 y1;

        public FirstOrderFilterQ31_32x64(double b0, double b1, double a0, double a1)
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
            this.x1 = 0;
            this.y1 = 0;
        }

        private FirstOrderFilterQ31_32x64(FirstOrderFilterQ31_32x64 other)
        {
            this.a1 = other.a1;
            this.b0 = other.b0;
            this.b1 = other.b1;
            this.postShift = other.postShift;
            this.x1 = other.x1;
            this.y1 = other.y1;
        }

        public IFilter<Int32> Clone()
        {
            return new FirstOrderFilterQ31_32x64(this);
        }

        public Int32 Filter(Int32 sample)
        {
            Int64 accumulator = this.b0 * sample + this.b1 * this.x1 - this.a1 * this.y1;
            this.x1 = sample;
            this.y1 = accumulator << this.postShift + 1;
            return (Int32)(accumulator >> Q31.MaximumFractionalBits - this.postShift);
        }
    }
}