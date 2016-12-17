using System;

namespace CrossTimeDsp.Dsp
{
    internal class Q31
    {
        public int FractionalBits { get; private set; }
        private Int32 value;

        public static readonly int MaximumFractionalBits = 31;

        public Q31(double value)
            : this(value, Q31.GetOptimalNumberOfFractionalBits(value))
        {
        }

        public Q31(double value, int fractionalBits)
        {
            this.FractionalBits = fractionalBits;
            this.value = (Int32)(Math.Pow(2.0, (double)fractionalBits) * value);
        }

        public static int GetOptimalNumberOfFractionalBits(double value)
        {
            int integerBits = Math.Max(0, (int)Math.Ceiling(Math.Log(Math.Abs(value), 2.0) + 10 * Double.Epsilon));
            if (integerBits > Q31.MaximumFractionalBits)
            {
                throw new ArgumentOutOfRangeException("value", String.Format("{0} is too large to fit in a 32 bit signed integer.", value));
            }
            return Q31.MaximumFractionalBits - integerBits;
        }

        public static int GetOptimalNumberOfFractionalBits(params double[] values)
        {
            int largestCommonNumberOfFractionalBits = Q31.MaximumFractionalBits;
            foreach (double value in values)
            {
                largestCommonNumberOfFractionalBits = Math.Min(Q31.GetOptimalNumberOfFractionalBits(value), largestCommonNumberOfFractionalBits);
            }
            return largestCommonNumberOfFractionalBits;
        }

        public static int GetQ31_32x64_PostShift(params double[] values)
        {
            int fractionalBitsInCoefficients = Q31.GetOptimalNumberOfFractionalBits(values);
            int postShift = Q31.MaximumFractionalBits - fractionalBitsInCoefficients;
            return postShift;
        }

        /// <summary>
        /// Performs a 32x32 bit multiply with 64 bit accumulation.  Post shifting is the caller's responsibility.
        /// </summary>
        public static Int64 operator *(Q31 x, Int32 y)
        {
            return (Int64)x.value * (Int64)y;
        }

        /// <summary>
        /// Performs a 32x64 bit multiply with 64 bit accumulation, ignoring the lowest 32 bits of the multiply.  Post shifting is the 
        /// caller's responsibility.
        /// </summary>
        public static Int64 operator *(Q31 x, Int64 y)
        {
            ////   all 64 bits of the upper multiply + the upper 32 bits of the lower multiply = Q95 intermediate result truncated to Q63
            return (Int64)x.value * (Int64)(y >> 32) + (((Int64)x.value * (y & 0x00000000FFFFFFFF)) >> 32);
        }
    }
}
