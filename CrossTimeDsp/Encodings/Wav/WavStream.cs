using CrossTimeDsp.Dsp;
using System;
using System.Collections.Generic;
using System.Diagnostics.CodeAnalysis;
using System.IO;

namespace CrossTimeDsp.Encodings.Wav
{
    /// <summary>
    /// A basic implementation of WAVE support per http://www-mmsp.ece.mcgill.ca/Documents/AudioFormats/WAVE/WAVE.html.
    /// </summary>
    [SuppressMessage("StyleCop.CSharp.DocumentationRules", "SA1650:ElementDocumentationMustBeSpelledCorrectly", Justification = "Url.")]
    public class WavStream : SampledStream
    {
        public string Artist { get; set; }
        public ChannelMask ChannelMask { get; set; }
        public string Genre { get; set; }
        public string Name { get; set; }
        public string Product { get; set; }
        public UInt32 SamplesPerChannel { get; set; }
        public UInt32 TotalSamples { get; set; }
        public string Track { get; set; }

        [SuppressMessage("StyleCop.CSharp.SpacingRules", "SA1002:SemicolonsMustBeSpacedCorrectly", Justification = "StyleCop limitation.")]
        public WavStream(string filePath)
        {
            this.Stream = new FileStream(filePath, FileMode.Open, FileAccess.Read, FileShare.Read);

            // read beginning of <WAVE-form>
            Chunk riff = this.ReadChunk();
            if (riff.ID != WavConstants.Riff)
            {
                throw new NotSupportedException(String.Format("Expected '{0}' to begin with 'RIFF' but got '{1}'.", filePath, riff.ID));
            }
            string wave = this.ReadAsciiString(WavConstants.ChunkIDSize);
            if (wave != WavConstants.Wave)
            {
                throw new NotSupportedException(String.Format("Expected the format of '{0}' to be 'WAVE' but got '{1}'.", filePath, wave));
            }
            // read <common-fields> portion of <fmt-ck>
            for (Chunk chunk = this.ReadChunk(); ; chunk = this.ReadChunk())
            {
                // read chunk
                switch (chunk.ID)
                {
                    case WavConstants.Chunks.Data:
                        // read beginning of <wave-data> chunk to position stream at start of sampled data and then return
                        // for accuracy, convert to bytes first before computing number of samples
                        this.TotalSamples = chunk.Size / (UInt32)(this.SampleSizeInBits / 8);
                        return;
                    case WavConstants.Chunks.Fact:
                        this.SamplesPerChannel = this.ReadUInt32();
                        break;
                    case WavConstants.Chunks.Format:
                        // three possibilities as to what this chunk contains, depending on the behavior of the code which created this .wav
                        // file
                        switch (chunk.Size)
                        {
                            case WavConstants.WavFormatExSize:
                                WaveFormatEx waveFormatEx = this.ReadStruct<WaveFormatEx>();
                                this.Channels = waveFormatEx.CommonFields.Channels;
                                this.SamplesPerSecond = (int)waveFormatEx.CommonFields.SamplesPerSecond;
                                this.SetSampleSizeInBits(waveFormatEx.CommonFields.BitsPerSample);
                                break;
                            case WavConstants.WavFormatExtensibleSize:
                                WaveFormatExtensible waveFormatExtensible = this.ReadStruct<WaveFormatExtensible>();
                                this.ChannelMask = waveFormatExtensible.ChannelMask;
                                this.Channels = waveFormatExtensible.Format.CommonFields.Channels;
                                this.SamplesPerSecond = (int)waveFormatExtensible.Format.CommonFields.SamplesPerSecond;
                                this.SetSampleSizeInBits(waveFormatExtensible.Format.CommonFields.BitsPerSample);
                                break;
                            // defined as <common-fields> in the WAVE 1.0 spec but usually implemented as a struct called WAVEFORMAT
                            case WavConstants.WavFormatSize:
                                WaveFormat waveFormat = this.ReadStruct<WaveFormat>();
                                this.Channels = waveFormat.Channels;
                                this.SamplesPerSecond = (int)waveFormat.SamplesPerSecond;
                                this.SetSampleSizeInBits(waveFormat.BitsPerSample);
                                break;
                            default:
                                throw new NotSupportedException(String.Format("Chunk '{0}' has a size of {1} bytes.  This does not match any of the known WAVEFORMAT structures.", WavConstants.Chunks.Format, chunk.Size));
                        }
                        break;
                    case WavConstants.Chunks.Artist:
                        this.Artist = this.ReadAsciiString((int)chunk.Size);
                        break;
                    case WavConstants.Chunks.Genre:
                        this.Genre = this.ReadAsciiString((int)chunk.Size);
                        break;
                    case WavConstants.Chunks.Name:
                        this.Name = this.ReadAsciiString((int)chunk.Size);
                        break;
                    case WavConstants.Chunks.Product:
                        this.Product = this.ReadAsciiString((int)chunk.Size);
                        break;
                    case WavConstants.Chunks.Itoc:
                        // this chunk does not seem to be documented, so skip it as required in the .wav file specification
                        this.Stream.Seek(chunk.Size, SeekOrigin.Current);
                        break;
                    case WavConstants.Chunks.Track:
                        this.Track = this.ReadAsciiString((int)chunk.Size);
                        break;
                    case WavConstants.Chunks.List:
                        string listType = this.ReadAsciiString(4);
                        switch (listType)
                        {
                            case WavConstants.Info:
                                // move on to reading next chunk
                                break;
                            default:
                                throw new NotSupportedException(String.Format("Unhandled list type '{0}'.", listType));
                        }
                        break;
                    default:
                        throw new NotSupportedException(String.Format("Unhandled chunk type '{0}'.", chunk.ID));
                }
                // now that chunk has been read, handle the chunk's word alignment padding byte if it exists (see definition of
                // ckData in the RIFF specification for details)
                if (chunk.Size % 2 == 1)
                {
                    this.Stream.Seek(1, SeekOrigin.Current);
                }
            }
        }

        public WavStream(string filePath, WavStream other, int sampleSizeInBits)
            : base(other)
        {
            // create output stream and intialize WavStream specific members in this instance
            this.Stream = new FileStream(filePath, FileMode.Create, FileAccess.Write);

            this.Artist = other.Artist;
            this.ChannelMask = other.ChannelMask;
            this.Genre = other.Genre;
            this.Name = other.Name;
            this.Product = other.Product;
            this.SetSampleSizeInBits(sampleSizeInBits);
            this.SamplesPerChannel = other.SamplesPerChannel;
            this.TotalSamples = other.TotalSamples;
            this.Track = other.Track;

            // write beginning of <WAVE-form>
            // size of file = 'WAVE' + list chunk if present + <fmt-ck> + <wave-data>
            UInt32 dataSize = this.GetDataChunkSize();
            UInt32 formatSize = this.GetFormatChunkSize();
            Nullable<UInt32> listSize = this.GetListChunkSize();
            UInt32 waveFormSize = (UInt32)(WavConstants.ChunkIDAndSizeSize + WavConstants.Riff.Length + WavConstants.ChunkIDAndSizeSize + formatSize + WavConstants.ChunkIDAndSizeSize) + dataSize;
            if (listSize.HasValue)
            {
                waveFormSize += WavConstants.ChunkIDAndSizeSize + listSize.Value;
            }
            Chunk waveForm = new Chunk(WavConstants.Riff, waveFormSize);
            this.WriteChunk(waveForm);
            this.WriteAsciiString(WavConstants.Wave);

            // write list chunk if file metadata is present
            if (listSize.HasValue)
            {
                Chunk list = new Chunk(WavConstants.Chunks.List, listSize.Value);
                this.WriteChunk(list);
                this.WriteAsciiString(WavConstants.Info);

                // write sub chunks in list
                if (String.IsNullOrEmpty(this.Artist) == false)
                {
                    Chunk artist = new Chunk(WavConstants.Chunks.Artist, this.Artist.Length);
                    this.WriteChunk(artist);
                    this.WriteAsciiString(this.Artist);
                    this.WritePaddingByteIfNeeded(artist);
                }
                if (String.IsNullOrEmpty(this.Genre) == false)
                {
                    Chunk genre = new Chunk(WavConstants.Chunks.Genre, this.Genre.Length);
                    this.WriteChunk(genre);
                    this.WriteAsciiString(this.Genre);
                    this.WritePaddingByteIfNeeded(genre);
                }
                if (String.IsNullOrEmpty(this.Name) == false)
                {
                    Chunk name = new Chunk(WavConstants.Chunks.Name, this.Name.Length);
                    this.WriteChunk(name);
                    this.WriteAsciiString(this.Name);
                    this.WritePaddingByteIfNeeded(name);
                }
                if (String.IsNullOrEmpty(this.Product) == false)
                {
                    Chunk product = new Chunk(WavConstants.Chunks.Product, this.Product.Length);
                    this.WriteChunk(product);
                    this.WriteAsciiString(this.Product);
                    this.WritePaddingByteIfNeeded(product);
                }
                if (String.IsNullOrEmpty(this.Track) == false)
                {
                    Chunk track = new Chunk(WavConstants.Chunks.Track, this.Track.Length);
                    this.WriteChunk(track);
                    this.WriteAsciiString(this.Track);
                    this.WritePaddingByteIfNeeded(track);
                }
            }

            // write <fmt-ck>
            Chunk format = new Chunk(WavConstants.Chunks.Format, formatSize);
            this.WriteChunk(format);
            if (this.GetOutputFormat() == FormatTag.WAVE_FORMAT_EXTENSIBLE)
            {
                WaveFormatExtensible wavFormatExtensible = new WaveFormatExtensible(this.Channels, this.SamplesPerSecond, this.SampleSizeInBits, this.ChannelMask);
                this.WriteStruct(wavFormatExtensible);
            }
            else
            {
                // it would be equally valid to write a WaveFormat struct
                WaveFormatEx wavFormatEx = new WaveFormatEx(this.Channels, this.SamplesPerSecond, this.SampleSizeInBits);
                this.WriteStruct(wavFormatEx);
            }
            this.WritePaddingByteIfNeeded(format);

            // write <fact-ck> if it's relevant
            if (this.GetOutputFormat() == FormatTag.WAVE_FORMAT_EXTENSIBLE)
            {
                Chunk fact = new Chunk(WavConstants.Chunks.Fact, sizeof(UInt32));
                this.WriteChunk(fact);
                this.WriteUInt32(this.SamplesPerChannel);
            }

            // write beginning of <wave-data> chunk to position stream at start of sampled data
            Chunk waveData = new Chunk(WavConstants.Chunks.Data, this.GetDataChunkSize());
            this.WriteChunk(waveData);
        }

        public override void Close()
        {
            // a padding byte is only needed in the somewhat obscure case of an odd number of channels, an odd number of samples, and
            // samples with an odd number of bytes per sample
            this.WritePaddingByteIfNeeded(this.GetDataChunkSize());
            base.Close();
        }

        private UInt32 GetDataChunkSize()
        {
            // avoid integer overflows by calculating the sample size in bytes before multiplying 
            // by the number of samples
            return this.TotalSamples * (UInt32)(this.SampleSizeInBits / 8);
        }

        private UInt32 GetFormatChunkSize()
        {
            int formatSize = WavConstants.WavFormatExSize;
            if (this.GetOutputFormat() == FormatTag.WAVE_FORMAT_EXTENSIBLE)
            {
                formatSize = WavConstants.WavFormatExtensibleSize;
            }
            return (UInt32)formatSize;
        }

        private Nullable<UInt32> GetListChunkSize()
        {
            // find size of list chunk if it were to be emitted, including any padding byte at the end
            int listSize = WavConstants.Chunks.List.Length;
            if (this.Artist != null)
            {
                listSize += WavConstants.ChunkIDAndSizeSize + this.Artist.Length + this.Artist.Length % WavConstants.ChunkAlignmentInBytes;
            }
            if (this.Genre != null)
            {
                listSize += WavConstants.ChunkIDAndSizeSize + this.Genre.Length + this.Genre.Length % WavConstants.ChunkAlignmentInBytes;
            }
            if (this.Name != null)
            {
                listSize += WavConstants.ChunkIDAndSizeSize + this.Name.Length + this.Name.Length % WavConstants.ChunkAlignmentInBytes;
            }
            if (this.Product != null)
            {
                listSize += WavConstants.ChunkIDAndSizeSize + this.Product.Length + this.Product.Length % WavConstants.ChunkAlignmentInBytes;
            }
            if (this.Track != null)
            {
                listSize += WavConstants.ChunkIDAndSizeSize + this.Track.Length + this.Track.Length % WavConstants.ChunkAlignmentInBytes;
            }

            // no need to emit list chunk if it doesn't have any content
            if (listSize == WavConstants.Chunks.List.Length)
            {
                return null;
            }
            return (UInt32)listSize;
        }

        public Dictionary<MetadataType, string> GetMetadata()
        {
            Dictionary<MetadataType, string> tags = new Dictionary<MetadataType, string>();
            if (String.IsNullOrEmpty(this.Artist) == false)
            {
                tags.Add(MetadataType.Artist, this.RemoveTrailingNullCharacter(this.Artist));
            }
            if (String.IsNullOrEmpty(this.Genre) == false)
            {
                tags.Add(MetadataType.Genre, this.RemoveTrailingNullCharacter(this.Genre));
            }
            if (String.IsNullOrEmpty(this.Name) == false)
            {
                tags.Add(MetadataType.Title, this.RemoveTrailingNullCharacter(this.Name));
            }
            if (String.IsNullOrEmpty(this.Product) == false)
            {
                tags.Add(MetadataType.Album, this.RemoveTrailingNullCharacter(this.Product));
            }
            if (String.IsNullOrEmpty(this.Track) == false)
            {
                tags.Add(MetadataType.Track, this.RemoveTrailingNullCharacter(this.Track));
            }
            return tags;
        }

        private FormatTag GetOutputFormat()
        {
            if (this.SampleSizeInBits == 16)
            {
                return FormatTag.WAVE_FORMAT_PCM;
            }
            return FormatTag.WAVE_FORMAT_EXTENSIBLE;
        }

        private Chunk ReadChunk()
        {
            Chunk chunk = new Chunk();
            chunk.ID = this.ReadAsciiString(WavConstants.ChunkIDSize);
            chunk.Size = this.ReadUInt32();
            return chunk;
        }

        private string RemoveTrailingNullCharacter(string tag)
        {
            return tag.Substring(0, tag.Length - 1);
        }

        private void WriteChunk(Chunk chunk)
        {
            this.WriteAsciiString(chunk.ID);
            this.WriteUInt32(chunk.Size);
        }

        private void WritePaddingByteIfNeeded(Chunk chunk)
        {
            if (chunk.Size % WavConstants.ChunkAlignmentInBytes == 1)
            {
                this.Stream.WriteByte(0);
            }
        }

        private void WritePaddingByteIfNeeded(UInt32 chunkSize)
        {
            if (chunkSize % WavConstants.ChunkAlignmentInBytes == 1)
            {
                this.Stream.WriteByte(0);
            }
        }
    }
}
