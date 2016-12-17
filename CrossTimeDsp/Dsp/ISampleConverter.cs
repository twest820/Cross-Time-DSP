using System;

namespace CrossTimeDsp.Dsp
{
    internal interface ISampleConverter<TSample> where TSample : struct
    {
        TSample BytesToSample(byte[] bytes);
        byte[] SampleToBytes(TSample sample);
        Int32 SampleToQ31(TSample sample);
    }
}
