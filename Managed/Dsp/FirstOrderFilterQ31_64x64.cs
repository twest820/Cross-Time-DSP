using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace CrossTimeDsp.Dsp
{
    /// <summary>
    /// direct form 1 implementation of a first order filter using Q63 coefficients and a Q63 state variable
    /// </summary>
    class FirstOrderFilterQ31_64x64 : IFilter<Int32>
    {
        private Q63 a1;
        private Q63 b0;
        private Q63 b1;
        private int postShift;
        private Int32 x1;
        private Int64 y1;

        public FirstOrderFilterQ31_64x64(double b0, double b1, double a0, double a1)
        {
            a1 /= a0;
            b0 /= a0;
            b1 /= a0;
            // see remarks in BiquadQ31_64x64..ctor()
            this.postShift = Q63.GetQ31_64x64_PostShift(a1, b0, b1);
            double coefficientScaling = Math.Pow(2.0, -this.postShift);
            this.a1 = new Q63(coefficientScaling * a1, Q63.MaximumFractionalBits);
            this.b0 = new Q63(coefficientScaling * b0, Q63.MaximumFractionalBits);
            this.b1 = new Q63(coefficientScaling * b1, Q63.MaximumFractionalBits);
            this.x1 = 0;
            this.y1 = 0;
        }

        private FirstOrderFilterQ31_64x64(FirstOrderFilterQ31_64x64 other)
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
            return new FirstOrderFilterQ31_64x64(this);
        }

        public Int32 Filter(Int32 sample)
        {
            Int64 accumulator = this.b0 * sample + this.b1 * this.x1 - this.a1 * this.y1;
            this.x1 = sample;
            this.y1 = accumulator << this.postShift + 1;
            // see remarks in BiquadQ31_64x64.Filter()
            return (Int32)(accumulator >> Q31.MaximumFractionalBits - this.postShift);
        }
    }
}