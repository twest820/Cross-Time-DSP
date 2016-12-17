using System;
using System.Collections;
using System.Collections.Generic;
using System.IO;
using System.Runtime.InteropServices;
using System.Text;

namespace CrossTimeDsp.Dsp
{
    // some of the read and write primitives in this class overlap with BinaryReader and BinaryWriter
    // if more primitives are needed it will become simpler to use the reader and writer rather than recode the functions
    public class SampledStream : IEnumerable<Int32>
    {
        private ISampleConverter<Int32> sampleConverter;
        private int sampleSizeInBits;

        protected Stream Stream { get; set; }

        public int Channels { get; protected set; }
        public int SamplesPerSecond { get; protected set; }

        protected SampledStream()
        {
        }

        protected SampledStream(SampledStream other)
        {
            // initialize values on this instance
            // call into SetSampleSizeInBits() as memberwise clone of convertFromDouble and convertToDouble would leave them pointing
            // to the other SampledStream rather than this instance
            this.Channels = other.Channels;
            this.SetSampleSizeInBits(other.SampleSizeInBits);
            this.SamplesPerSecond = other.SamplesPerSecond;
        }

        public int SampleSizeInBits
        {
            get { return this.sampleSizeInBits; }
        }

        public virtual void Close()
        {
            this.Stream.Close();
        }

        /// <summary>
        /// returns an enumerator of samples in Q31 format
        /// </summary>
        public IEnumerator<Int32> GetEnumerator()
        {
            byte[] rawData = new byte[this.SampleSizeInBits / 8];
            for (int bytesRead = this.Stream.Read(rawData, 0, rawData.Length); bytesRead == rawData.Length; bytesRead = this.Stream.Read(rawData, 0, rawData.Length))
            {
                yield return this.sampleConverter.BytesToSample(rawData);
            }
        }

        IEnumerator IEnumerable.GetEnumerator()
        {
            return this.GetEnumerator();
        }

        protected string ReadAsciiString(int length)
        {
            byte[] rawData = new byte[length];
            this.Stream.Read(rawData, 0, rawData.Length);
            return Encoding.ASCII.GetString(rawData);
        }

        protected TStruct ReadStruct<TStruct>() where TStruct : struct
        {
            byte[] structAsBytes = new byte[Marshal.SizeOf(typeof(TStruct))];
            this.Stream.Read(structAsBytes, 0, structAsBytes.Length);
            GCHandle pinnedStruct = GCHandle.Alloc(structAsBytes, GCHandleType.Pinned);
            TStruct structure = (TStruct)Marshal.PtrToStructure(pinnedStruct.AddrOfPinnedObject(), typeof(TStruct));
            pinnedStruct.Free();
            return structure;
        }

        protected UInt32 ReadUInt32()
        {
            byte[] integerAsBytes = new byte[sizeof(UInt32)];
            this.Stream.Read(integerAsBytes, 0, integerAsBytes.Length);
            return BitConverter.ToUInt32(integerAsBytes, 0);
        }

        protected void SetSampleSizeInBits(int sampleSizeInBits)
        {
            // if needed, add support data formats with unsigned samples
            switch (sampleSizeInBits)
            {
                case 16:
                    this.sampleConverter = new Int16SampleConverter();
                    break;
                case 24:
                    this.sampleConverter = new Int24SampleConverter();
                    break;
                default:
                    throw new NotSupportedException(String.Format("Unhandled sample size of {0} bits.", sampleSizeInBits));
            }
            this.sampleSizeInBits = sampleSizeInBits;
        }

        protected void WriteAsciiString(string str)
        {
            byte[] rawData = Encoding.ASCII.GetBytes(str);
            this.Stream.Write(rawData, 0, rawData.Length);
        }

        /// <summary>
        /// write a sample to the stream
        /// </summary>
        /// <param name="sample">sample in Q31 format</param>
        public void WriteSample(Int32 sample)
        {
            byte[] rawData = this.sampleConverter.SampleToBytes(sample);       
            this.Stream.Write(rawData, 0, rawData.Length);
        }

        protected void WriteStruct<TStruct>(TStruct structure) where TStruct : struct
        {
            byte[] structAsBytes = new byte[Marshal.SizeOf(typeof(TStruct))];
            GCHandle pinnedStruct = GCHandle.Alloc(structAsBytes, GCHandleType.Pinned);
            Marshal.StructureToPtr(structure, pinnedStruct.AddrOfPinnedObject(), false);
            this.Stream.Write(structAsBytes, 0, structAsBytes.Length);
            pinnedStruct.Free();
        }

        protected void WriteUInt32(UInt32 integer)
        {
            byte[] rawData = BitConverter.GetBytes(integer);
            this.Stream.Write(rawData, 0, rawData.Length);
        }
    }
}
