using System;
using System.Collections;
using System.Collections.Generic;

namespace CrossTimeDsp.Dsp
{
    internal class SampledStreamConverter<TSample> : IEnumerable<TSample> where TSample : struct
    {
        private IEnumerable<Int32> sampledStream;

        public SampledStreamConverter(SampledStream sampledStream)
        {
            this.sampledStream = sampledStream;
        }

        public IEnumerator<TSample> GetEnumerator()
        {
            return (IEnumerator<TSample>)new DoubleSampleEnumerator(this.sampledStream.GetEnumerator());
        }

        IEnumerator IEnumerable.GetEnumerator()
        {
            return this.GetEnumerator();
        }
    }
}
