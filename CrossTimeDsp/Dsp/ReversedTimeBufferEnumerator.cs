using System;
using System.Collections;
using System.Collections.Generic;

namespace CrossTimeDsp.Dsp
{
    internal class ReversedTimeBufferEnumerator<TSample> : IEnumerator<TSample> where TSample : struct
    {
        private ReversedTimeBuffer<TSample> buffer;
        private Int64 index;

        public ReversedTimeBufferEnumerator(ReversedTimeBuffer<TSample> buffer)
        {
            this.buffer = buffer;
            this.index = -1;
        }

        public TSample Current
        {
            get { return this.buffer[(uint)this.index]; }
        }

        object IEnumerator.Current
        {
            get { return this.Current; }
        }

        public void Dispose()
        {
            this.buffer = null;
        }

        public bool MoveNext()
        {
            ++this.index;
            return this.index < (Int64)this.buffer.TotalSamples;
        }

        public void Reset()
        {
            this.index = 0;
        }
    }
}
