using NAudio.Wave;
using System;
using System.Collections.Generic;
using System.IO;
using System.Runtime.Remoting;
using System.Threading;
using System.Threading.Tasks;

namespace CrossTimeDsp.Dsp
{
    internal class SampleBuffer : WaveStream
    {
        private LinkedListNode<SampleBlock> currentBlockNode;
        private long length;
        private long position;
        private int positionInBlock;
        private WaveFormat waveFormat;

        public LinkedList<SampleBlock> Blocks { get; private set; }

        public override long Position
        {
            get { return this.position; }
            set { this.position = value; }
        }

        public SampleBuffer(int sampleRate, int bitsPerSample, int channels)
        {
            this.currentBlockNode = null;
            this.length = 0;
            this.position = 0;
            this.waveFormat = new WaveFormat(sampleRate, bitsPerSample, channels);

            this.Blocks = new LinkedList<SampleBlock>();
        }

        public SampleBuffer(WaveStream stream)
            : this(stream.WaveFormat.SampleRate, stream.WaveFormat.BitsPerSample, stream.WaveFormat.Channels)
        {
            SampleType sampleType = SampleTypeExtensions.FromBitsPerSample(stream.WaveFormat.BitsPerSample);
            while (stream.CanRead)
            {
                SampleBlock block = new SampleBlock(Constant.SampleBlockSizeInBytes, sampleType);
                block.BytesInUse = stream.Read(block.ByteBuffer, 0, block.MaximumSizeInBytes);
                if (block.BytesInUse < 1)
                {
                    // workaround for NAudio bug: WaveStream.CanRead is hard coded to true regardless of position
                    break;
                }

                this.Blocks.AddLast(block);
                this.length += block.BytesInUse;
            }
        }

        public override bool CanSeek
        {
            get { return false; }
        }

        public override bool CanRead
        {
            get { return this.position < this.length; }
        }

        public override bool CanTimeout
        {
            get { return false; }
        }

        public override bool CanWrite
        {
            get { return false; }
        }

        public override long Length
        {
            get { return this.length; }
        }

        public override int ReadTimeout
        {
            get { throw new NotImplementedException(); }
            set { throw new NotImplementedException(); }
        }

        public override WaveFormat WaveFormat
        {
            get { return this.waveFormat; }
        }

        public override int WriteTimeout
        {
            get { throw new NotImplementedException(); }
            set { throw new NotImplementedException(); }
        }

        public override IAsyncResult BeginRead(byte[] buffer, int offset, int count, AsyncCallback callback, object state)
        {
            throw new NotImplementedException();
        }

        public override IAsyncResult BeginWrite(byte[] buffer, int offset, int count, AsyncCallback callback, object state)
        {
            throw new NotImplementedException();
        }

        public override void Close()
        {
            throw new NotImplementedException();
        }

        public override Task CopyToAsync(Stream destination, int bufferSize, CancellationToken cancellationToken)
        {
            throw new NotImplementedException();
        }

        public override ObjRef CreateObjRef(Type requestedType)
        {
            throw new NotImplementedException();
        }

        public override int EndRead(IAsyncResult asyncResult)
        {
            throw new NotImplementedException();
        }

        public override void EndWrite(IAsyncResult asyncResult)
        {
            throw new NotImplementedException();
        }

        public override void Flush()
        {
            throw new NotImplementedException();
        }

        public override Task FlushAsync(CancellationToken cancellationToken)
        {
            throw new NotImplementedException();
        }

        public override int Read(byte[] buffer, int offset, int count)
        {
            if (this.currentBlockNode == null || this.currentBlockNode.Value == null)
            {
                return 0;
            }

            SampleBlock block = this.currentBlockNode.Value;
            int bytesAvailableInCurrentBlock = block.BytesInUse - this.positionInBlock;
            int bytesCopied = Math.Min(bytesAvailableInCurrentBlock, count);
            Array.Copy(block.ByteBuffer, this.positionInBlock, buffer, offset, bytesCopied);
            this.positionInBlock += bytesCopied;
            this.position += bytesCopied;

            if (block.BytesInUse <= this.positionInBlock)
            {
                this.currentBlockNode = this.currentBlockNode.Next;
                this.positionInBlock = 0;
                int bytesRemainingToCopy = count - bytesAvailableInCurrentBlock;
                if (bytesRemainingToCopy > 0)
                {
                    bytesCopied += this.Read(buffer, offset + bytesCopied, bytesRemainingToCopy);
                }
            }

            return bytesCopied;
        }

        public override Task<int> ReadAsync(byte[] buffer, int offset, int count, CancellationToken cancellationToken)
        {
            throw new NotImplementedException();
        }

        public override int ReadByte()
        {
            throw new NotImplementedException();
        }

        public void RecalculateBlocks()
        {
            this.currentBlockNode = this.Blocks.First;
            this.length = 0;
            foreach (SampleBlock block in this.Blocks)
            {
                this.length += block.BytesInUse;
            }
            this.positionInBlock = 0;
        }

        public override long Seek(long offset, SeekOrigin origin)
        {
            throw new NotImplementedException();
        }

        public override void SetLength(long length)
        {
            throw new NotImplementedException();
        }

        public override void Write(byte[] buffer, int offset, int count)
        {
            throw new NotImplementedException();
        }

        public override Task WriteAsync(byte[] buffer, int offset, int count, CancellationToken cancellationToken)
        {
            throw new NotImplementedException();
        }

        public override void WriteByte(byte value)
        {
            throw new NotImplementedException();
        }
    }
}
