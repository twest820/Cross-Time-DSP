using System;

namespace CrossTimeDsp.Dsp
{
    internal class Q63
    {
        public int FractionalBits { get; private set; }
        private Int64 value;

        public static readonly int MaximumFractionalBits = 63;

        public Q63(double value)
            : this(value, Q63.GetOptimalNumberOfFractionalBits(value))
        {
        }

        public Q63(double value, int fractionalBits)
        {
            this.FractionalBits = fractionalBits;
            this.value = (Int64)(Math.Pow(2.0, (double)fractionalBits) * value);
        }

        public static int GetOptimalNumberOfFractionalBits(double value)
        {
            int integerBits = Math.Max(0, (int)Math.Ceiling(Math.Log(Math.Abs(value), 2.0) + 10 * Double.Epsilon));
            if (integerBits > Q63.MaximumFractionalBits)
            {
                throw new ArgumentOutOfRangeException("value", String.Format("{0} is too large to fit in a 32 bit signed integer.", value));
            }
            return Q63.MaximumFractionalBits - integerBits;
        }

        public static int GetOptimalNumberOfFractionalBits(params double[] values)
        {
            int largestCommonNumberOfFractionalBits = Q63.MaximumFractionalBits;
            foreach (double value in values)
            {
                largestCommonNumberOfFractionalBits = Math.Min(Q63.GetOptimalNumberOfFractionalBits(value), largestCommonNumberOfFractionalBits);
            }
            return largestCommonNumberOfFractionalBits;
        }

        public static int GetQ31_64x64_PostShift(params double[] values)
        {
            int fractionalBitsInCoefficients = Q63.GetOptimalNumberOfFractionalBits(values);
            int postShift = Q63.MaximumFractionalBits - fractionalBitsInCoefficients;
            return postShift;
        }

        /// <summary>
        /// Performs a 64x32 bit multiply with 64 bit accumulation, ignoring the lowest 32 bits of the multiply.  Post shifting is the 
        /// caller's responsibility.
        /// </summary>
        public static Int64 operator *(Q63 x, Int32 y)
        {
            ////   all 64 bits of the upper multiply + the upper 32 bits of the lower multiply = Q95 intermediate result truncated to Q63
            return (x.value >> 32) * (Int64)y + ((x.value & 0x00000000ffffffff) * (Int64)y >> 32);
        }

        /// <summary>
        /// Performs a 64x64 bit multiply with 64 bit accumulation, ignoring the lowest 64 bits of the multiply.  Post shifting is the 
        /// caller's responsibility.
        /// </summary>
        public static Int64 operator *(Q63 x, Int64 y)
        {
            ////   all 64 bits of the upper multiply + the upper 32 bits of the lower multiplies = Q127 intermediate result truncated to Q63
            return (x.value >> 32) * (y >> 32) + ((x.value & 0x00000000ffffffff) * (y >> 32) >> 32) + ((x.value >> 32) * (y & 0x00000000FFFFFFFF) >> 32);
        }
    }
}
