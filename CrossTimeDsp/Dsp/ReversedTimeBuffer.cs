using System;
using System.Collections;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Runtime.InteropServices;

namespace CrossTimeDsp.Dsp
{
    internal class ReversedTimeBuffer<TSample> : IDisposable, IEnumerable<TSample> where TSample : struct
    {
        private static readonly Int64 MaxInMemorySamples = 1024 * 1024 * 1024 / Marshal.SizeOf(typeof(TSample)); // 1GB of samples

        private TSample[] buffer;
        private Int64 firstSampleInBuffer;
        private ISampleConverter<TSample> sampleConverter;
        private string temporaryFileName;
        private BinaryReader temporaryFileReader;
        private Stream temporaryFileStream;
        private BinaryWriter temporaryFileWriter;
        private Int64 totalSamples;

        public ReversedTimeBuffer(IEnumerable<TSample> samples, Int64 totalSamples)
        {
            if (totalSamples < ReversedTimeBuffer<TSample>.MaxInMemorySamples)
            {
                this.buffer = new TSample[totalSamples];
            }
            else
            {
                this.buffer = new TSample[ReversedTimeBuffer<TSample>.MaxInMemorySamples];
                if (typeof(TSample) == typeof(double))
                {
                    this.sampleConverter = (ISampleConverter<TSample>)new DoubleSampleConverter();
                }
                else if (typeof(TSample) == typeof(Int32))
                {
                    this.sampleConverter = (ISampleConverter<TSample>)new Int32SampleConverter();
                }
                this.temporaryFileName = Path.GetTempFileName();
                this.temporaryFileStream = new FileStream(this.temporaryFileName, FileMode.Create);
                this.temporaryFileReader = new BinaryReader(this.temporaryFileStream);
                this.temporaryFileWriter = new BinaryWriter(this.temporaryFileStream);
            }
            this.firstSampleInBuffer = 0;
            this.totalSamples = totalSamples;

            // in priciple this loop could be implemented with a foreach (TSample sample in samples) but the size field of some .wav files' 
            // data chunk lists a size smaller than the actual size of the chunk, resulting in reads from samples eventually overflowing
            // the buffer in the in memory case
            IEnumerator<TSample> samplesEnumerator = samples.GetEnumerator();
            for (uint sample = 0; sample < totalSamples; ++sample)
            {
                if (samplesEnumerator.MoveNext() == false)
                {
                    // if needed, handle this case more gracefully
                    throw new ApplicationException(String.Format("{0} samples were expected to be present but only {1} were found in the input file.", totalSamples, sample));
                }
                this[sample] = samplesEnumerator.Current;
            }
            if (samplesEnumerator.MoveNext())
            {
                Trace.TraceWarning("{0} samples were expected to be present but file contains more than that.  Truncation will occur.", totalSamples);
            }
        }

        public Int64 TotalSamples
        {
            get { return this.totalSamples; }
        }

        public TSample this[Int64 index]
        {
            get
            {
                this.SynchronizeBuffer(index);
                return this.buffer[index - this.firstSampleInBuffer];
            }
            set 
            {
                this.SynchronizeBuffer(index);
                this.buffer[index - this.firstSampleInBuffer] = value;
            }
        }

        public IEnumerator<TSample> GetEnumerator()
        {
            return new ReversedTimeBufferEnumerator<TSample>(this);
        }

        IEnumerator IEnumerable.GetEnumerator()
        {
            return this.GetEnumerator();
        }

        void IDisposable.Dispose()
        {
            if (this.temporaryFileName != null)
            {
                this.temporaryFileStream.Dispose();
            }
        }

        private void SynchronizeBuffer(Int64 index)
        {
            // nothing to do if sample is already in memory
            if (this.firstSampleInBuffer <= index && index < this.firstSampleInBuffer + this.buffer.Length)
            {
                return;
            }
            // if sample is not in memory, verify the index is valid
            // this prevents various errors, such as trying to read from a nonexistent temp file on disk when only in memory buffering 
            // is being done but a caller requests a sample beyond the size of the buffer
            if (index < 0 || index > this.totalSamples - 1)
            {
                throw new ArgumentOutOfRangeException("index", String.Format("{0} is not between 0 and {1}.", index, this.totalSamples - 1));
            }

            // write current buffer to disk and then load the new buffer to retrieve the requested sample
            // write current buffer to disk
            int sampleSizeInBytes = Marshal.SizeOf(typeof(TSample));
            this.temporaryFileStream.Seek(this.firstSampleInBuffer * (Int64)sampleSizeInBytes, SeekOrigin.Begin);
            for (UInt32 writeIndex = 0; writeIndex < this.buffer.Length; ++writeIndex)
            {
                byte[] sample = this.sampleConverter.SampleToBytes(this.buffer[writeIndex]);
                this.temporaryFileWriter.Write(sample);
            }
            this.temporaryFileWriter.Flush();

            // compute start index of new buffer
            this.firstSampleInBuffer = index - index % (uint)this.buffer.Length;

            // load buffer
            UInt32 samplesToRead = (UInt32)Math.Min(this.temporaryFileStream.Length - this.firstSampleInBuffer * (Int64)sampleSizeInBytes, this.buffer.Length);
            if (samplesToRead > 0)
            {
                this.temporaryFileStream.Seek(this.firstSampleInBuffer * (Int64)sampleSizeInBytes, SeekOrigin.Begin);
                for (UInt32 readIndex = 0; readIndex < samplesToRead; ++readIndex)
                {
                    byte[] sample = this.temporaryFileReader.ReadBytes(sampleSizeInBytes);
                    this.buffer[readIndex] = this.sampleConverter.BytesToSample(sample);
                }
            }
        }
    }
}
