using System;
using System.Collections;
using System.Collections.Generic;

namespace CrossTimeDsp.Dsp
{
    internal class DoubleSampleEnumerator : IEnumerator<double>
    {
        private IEnumerator<Int32> q31SampleEnumerator;

        public DoubleSampleEnumerator(IEnumerator<Int32> q31SampleEnumerator)
        {
            this.q31SampleEnumerator = q31SampleEnumerator;
        }

        public double Current
        {
            get { return (double)this.q31SampleEnumerator.Current; }
        }

        public void Dispose()
        {
            // nothing to do
        }

        object IEnumerator.Current
        {
            get { return this.Current; }
        }

        public bool MoveNext()
        {
            return this.q31SampleEnumerator.MoveNext();
        }

        public void Reset()
        {
            this.q31SampleEnumerator.Reset();
        }
    }
}
