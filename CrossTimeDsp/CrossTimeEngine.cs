using CrossTimeDsp.Configuration;
using CrossTimeDsp.Dsp;
using NAudio.MediaFoundation;
using NAudio.Wave;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Threading.Tasks;
using TagLib;
using File = System.IO.File;
using TagFile = TagLib.File;

namespace CrossTimeDsp
{
    internal class CrossTimeEngine : IDisposable
    {
        private const double ClipWarningThreshold = 0.01;
        private static readonly TimeSpan FileDeletionRetryInterval = TimeSpan.FromMilliseconds(250);

        private bool disposed;
        private CrossTimeEngineLog log;

        public CrossTimeDspConfiguration Configuration { get; private set; }
        public bool Stopping { get; set; }

        public CrossTimeEngine(string configurationFilePath, CrossTimeEngineLog log)
        {
            this.Configuration = CrossTimeDspConfiguration.Load(configurationFilePath);
            this.disposed = false;
            this.log = log;
            this.Stopping = false;

            MediaFoundationApi.Startup();

            Filter firstReverseTimeFilter = this.Configuration.Filters.FirstOrDefault(filter => filter.TimeDirection == TimeDirection.Reverse);
            if (firstReverseTimeFilter != null)
            {
                firstReverseTimeFilter.AdjustGain(this.Configuration.Engine.ReverseTimeAntiClippingAttenuationInDB);
            }
        }

        public void Dispose()
        {
            this.Dispose(true);
            GC.SuppressFinalize(this);
        }

        protected virtual void Dispose(bool disposing)
        {
            if (this.disposed)
            {
                return;
            }

            if (disposing)
            {
                MediaFoundationApi.Shutdown();
            }

            this.disposed = true;
        }

        private void FilterFile(string inputFilePath, string outputFilePath)
        {
            // nothing to do if the output file already exists and is newer than both the input file and Cross Time DSP's app.config file
            if (File.Exists(outputFilePath))
            {
                DateTime inputLastWriteTimeUtc = File.GetLastWriteTimeUtc(inputFilePath);
                DateTime outputLastWriteTimeUtc = File.GetLastWriteTimeUtc(outputFilePath);
                if ((outputLastWriteTimeUtc > inputLastWriteTimeUtc) && 
                    (outputLastWriteTimeUtc > this.Configuration.LastWriteTimeUtc))
                {
                    this.log.ReportVerbose("'{0}': skipped as '{1}' is newer.", Path.GetFileName(inputFilePath), Path.GetFileName(outputFilePath));
                    return;
                }
            }

            // get input
            DateTime processingStartedUtc = DateTime.UtcNow;
            MediaFoundationReader inputStream = new MediaFoundationReader(inputFilePath);
            if (Constant.SampleBlockSizeInBytes % inputStream.WaveFormat.BlockAlign != 0)
            {
                this.log.ReportError("'{0}': cannot be processed as sample block size of {0} bytes is not an exact multiple of the input block alignment of {1} bytes.", Path.GetFileName(inputFilePath), Constant.SampleBlockSizeInBytes, inputStream.WaveFormat.BlockAlign);
                return;
            }

            // ensure output directory exists so that output file write succeeds
            string outputDirectoryPath = Path.GetDirectoryName(outputFilePath);
            if (String.IsNullOrEmpty(outputDirectoryPath) == false && Directory.Exists(outputDirectoryPath) == false)
            {
                Directory.CreateDirectory(outputDirectoryPath);
            }

            StreamPerformance streamMetrics;
            using (WaveStream outputStream = this.FilterStream(inputStream, out streamMetrics))
            {
                // do the filtering
                if (this.Stopping)
                {
                    // if the stop flag was set during filtering outputStream will be null
                    return;
                }

                // write output file
                MediaType outputMediaType;
                if (this.Configuration.Output.Encoding == Encoding.Wave)
                {
                    // work around NAudio bug: MediaFoundationEncoder supports Wave files but GetOutputMediaTypes() fails on Wave
                    outputMediaType = new MediaType(outputStream.WaveFormat);
                }
                else
                {
                    List<MediaType> outputMediaTypes = MediaFoundationEncoder.GetOutputMediaTypes(Constant.EncodingGuids[this.Configuration.Output.Encoding]).Where(mediaType => mediaType.BitsPerSample == outputStream.WaveFormat.BitsPerSample && mediaType.ChannelCount == outputStream.WaveFormat.Channels && mediaType.SampleRate == outputStream.WaveFormat.SampleRate).ToList();
                    if ((outputMediaTypes == null) || (outputMediaTypes.Count < 1))
                    {
                        this.log.ReportError("'{0}': no media type found for {1} bits per sample, {2} channels, at {3} kHz.", Path.GetFileName(inputFilePath), outputStream.WaveFormat.BitsPerSample, outputStream.WaveFormat.Channels, outputStream.WaveFormat.SampleRate);
                        return;
                    }
                    outputMediaType = outputMediaTypes[0];
                }
                MediaFoundationEncoder outputEncoder = new MediaFoundationEncoder(outputMediaType);
                streamMetrics.EncodingStartedUtc = DateTime.UtcNow;
                outputEncoder.Encode(outputFilePath, outputStream);
                streamMetrics.EncodingStoppedUtc = DateTime.UtcNow;
            }

            // copy metadata
            Tag inputMetadata;
            using (FileStream inputMetadataStream = new FileStream(inputFilePath, FileMode.Open, FileAccess.Read, FileShare.Read))
            {
                using (TagFile inputTagFile = TagFile.Create(new StreamFileAbstraction(inputMetadataStream.Name, inputMetadataStream, inputMetadataStream)))
                {
                    inputMetadata = inputTagFile.Tag;
                }
            }

            using (FileStream outputMetadataStream = new FileStream(outputFilePath, FileMode.Open, FileAccess.ReadWrite, FileShare.Read))
            {
                using (TagFile outputTagFile = TagFile.Create(new StreamFileAbstraction(outputMetadataStream.Name, outputMetadataStream, outputMetadataStream)))
                {
                    if (this.TryApplyMetadata(inputMetadata, outputTagFile.Tag))
                    {
                        outputTagFile.Save();
                    }
                }
            }

            DateTime processingStoppedUtc = DateTime.UtcNow;
            TimeSpan encodingTime = streamMetrics.EncodingStoppedUtc - streamMetrics.EncodingStartedUtc;
            TimeSpan processingTime = processingStoppedUtc - processingStartedUtc;
            if (streamMetrics.HasReverseTime)
            {
                TimeSpan reverseBufferTime = streamMetrics.ReverseBufferCompleteUtc - streamMetrics.StartTimeUtc;
                TimeSpan reverseProcessingTime = streamMetrics.ReverseTimeCompleteUtc - streamMetrics.ReverseBufferCompleteUtc;
                if (streamMetrics.HasForwardTime)
                {
                    TimeSpan forwardProcessingTime = streamMetrics.CompleteTimeUtc - streamMetrics.ReverseTimeCompleteUtc;
                    this.log.ReportVerbose("'{0}' to '{1}' in {2} (buffer {3}, reverse {4}, forward {5}, encode {6}).", Path.GetFileName(inputFilePath), Path.GetFileName(outputFilePath), processingTime.ToString(Constant.ElapsedTimeFormat), reverseBufferTime.ToString(Constant.ElapsedTimeFormat), reverseProcessingTime.ToString(Constant.ElapsedTimeFormat), forwardProcessingTime.ToString(Constant.ElapsedTimeFormat), encodingTime.ToString(Constant.ElapsedTimeFormat));
                }
                else
                {
                    this.log.ReportVerbose("'{0}' to '{1}' in {2} (buffer {3}, reverse {4}, encode {5}).", Path.GetFileName(inputFilePath), Path.GetFileName(outputFilePath), processingTime.ToString(Constant.ElapsedTimeFormat), reverseBufferTime.ToString(Constant.ElapsedTimeFormat), reverseProcessingTime.ToString(Constant.ElapsedTimeFormat), encodingTime.ToString(Constant.ElapsedTimeFormat));
                }
            }
            else
            {
                TimeSpan filteringTime = streamMetrics.CompleteTimeUtc - streamMetrics.StartTimeUtc;
                this.log.ReportVerbose("'{0}' to '{1}' in {2} (load+filter {3}, encode {4}).", Path.GetFileName(inputFilePath), Path.GetFileName(outputFilePath), processingTime.ToString(Constant.ElapsedTimeFormat), filteringTime.ToString(Constant.ElapsedTimeFormat), encodingTime.ToString(Constant.ElapsedTimeFormat));
            }
        }

        public void FilterFiles(string inputDirectoryPath, string fileNameOrSearchPattern, string outputPath)
        {
            // trace directory which was just entered
            // this makes it easier to monitor progress when processing large numbers of artists and albums
            this.log.ReportVerbose("Processing '{0}'...", inputDirectoryPath);

            // process files in this directory matching the search pattern
            // Measurements show automatic parallelism is typically fastest; per track processing times are 20-70% higher than explicit specification but total
            // time is around 10% less.  It appears this is due to Windows Media Foundation performing encoding in parallel and automatic allocation being better 
            // at reducing contention between the encoder and Cross Time DSP worker threads, though there may be some contribution from NAudio's lack of async
            // support.  Another source of variability is the number of cores against the number and size of tracks to process; changing specification produces
            // different scheduling alignments, affecting when the last track completes relative to others.
            Parallel.ForEach(Directory.GetFiles(inputDirectoryPath, fileNameOrSearchPattern), (string inputFilePath, ParallelLoopState loopState) =>
            {
                if (this.Stopping)
                {
                    loopState.Stop();
                    return;
                }

                // process this input file
                string outputFilePath = this.GetOutputFilePath(inputFilePath, outputPath, this.Configuration.Output.Encoding);
                this.FilterFile(inputFilePath, outputFilePath);
            });

            // recurse through subdirectories
            foreach (string inputSubdirectoryPath in Directory.GetDirectories(inputDirectoryPath))
            {
                string inputSubdirectoryName = Path.GetFileName(inputSubdirectoryPath);
                this.FilterFiles(inputSubdirectoryPath, fileNameOrSearchPattern, Path.Combine(outputPath, inputSubdirectoryName));
            }
        }

        private WaveStream FilterStream(WaveStream inputStream, out StreamPerformance performance)
        {
            performance = new StreamPerformance();

            // populate filters
            FilterBank forwardTimeFilters = new FilterBank(this.Configuration.Engine.Precision, inputStream.WaveFormat.SampleRate, inputStream.WaveFormat.Channels, this.Configuration.Engine.Q31Adaptive.Q31_32x64_Threshold, this.Configuration.Engine.Q31Adaptive.Q31_64x64_Threshold);
            FilterBank reverseTimeFilters = new FilterBank(this.Configuration.Engine.Precision, inputStream.WaveFormat.SampleRate, inputStream.WaveFormat.Channels, this.Configuration.Engine.Q31Adaptive.Q31_32x64_Threshold, this.Configuration.Engine.Q31Adaptive.Q31_64x64_Threshold);
            foreach (Filter filter in this.Configuration.Filters)
            {
                switch (filter.TimeDirection)
                {
                    case TimeDirection.Forward:
                        filter.AddTo(forwardTimeFilters);
                        break;
                    case TimeDirection.Reverse:
                        filter.AddTo(reverseTimeFilters);
                        break;
                    default:
                        throw new NotSupportedException(String.Format("Unhandled time direction {0}.", filter.TimeDirection));
                }
            }

            // do reverse time pass
            // If the only reverse time filter is the anti-clipping gain then there's nothing to do.
            SampleType dataPathSampleType = this.Configuration.Engine.Precision == FilterPrecision.Double ? SampleType.Double : SampleType.Int32;
            bool hasForwardTimeFilters = forwardTimeFilters.FilterCount > 0;
            SampleType outputSampleType = SampleTypeExtensions.FromBitsPerSample(this.Configuration.Output.BitsPerSample);
            if (reverseTimeFilters.FilterCount > 0 && (this.Stopping == false))
            {
                using (SampleBuffer inputBuffer = new SampleBuffer(inputStream))
                {
                    // set up buffer for output of reverse time pass and dispose input stream as it's no longer needed
                    performance.ReverseBufferCompleteUtc = DateTime.UtcNow;
                    SampleType reverseTimeSampleType = hasForwardTimeFilters ? dataPathSampleType : outputSampleType;
                    SampleBuffer reverseTimeBuffer = new SampleBuffer(inputStream.WaveFormat.SampleRate, reverseTimeSampleType.BitsPerSample(), inputStream.WaveFormat.Channels);
                    inputStream.Dispose();

                    // input blocks are disposed as they're processed to limit peak memory consumption (and removed from the input buffer for completeness)
                    SampleBlock recirculatingDataPathBlock = null;
                    for (LinkedListNode<SampleBlock> blockNode = inputBuffer.Blocks.Last; blockNode != null; blockNode = blockNode.Previous, inputBuffer.Blocks.RemoveLast())
                    {
                        using (SampleBlock block = blockNode.Value)
                        {
                            SampleBlock filteredBlock = reverseTimeFilters.FilterReverse(block, dataPathSampleType, reverseTimeSampleType, ref recirculatingDataPathBlock);
                            reverseTimeBuffer.Blocks.AddFirst(filteredBlock);
                        }

                        if (this.Stopping)
                        {
                            break;
                        }
                    }
                    if (recirculatingDataPathBlock != null)
                    {
                        recirculatingDataPathBlock.Dispose();
                    }

                    // prepare to apply forward time pass to output of reverse time pass or just to write output
                    reverseTimeBuffer.RecalculateBlocks();
                    inputStream = reverseTimeBuffer;
                    performance.ReverseTimeCompleteUtc = DateTime.UtcNow;
                }
            }

            // do forward time pass
            if (hasForwardTimeFilters && (this.Stopping == false))
            {
                SampleBuffer outputStream = new SampleBuffer(inputStream.WaveFormat.SampleRate, outputSampleType.BitsPerSample(), inputStream.WaveFormat.Channels);
                SampleBlock recirculatingDataPathBlock = null;
                if (inputStream is SampleBuffer)
                {
                    SampleBuffer inputBlocks = (SampleBuffer)inputStream;
                    for (LinkedListNode<SampleBlock> blockNode = inputBlocks.Blocks.First; blockNode != null; blockNode = blockNode.Next, inputBlocks.Blocks.RemoveFirst())
                    {
                        using (SampleBlock block = blockNode.Value)
                        {
                            SampleBlock filteredBlock = forwardTimeFilters.Filter(block, dataPathSampleType, outputSampleType, ref recirculatingDataPathBlock);
                            outputStream.Blocks.AddLast(filteredBlock);
                        }

                        if (this.Stopping)
                        {
                            break;
                        }
                    }
                }
                else
                {
                    byte[] inputBuffer = new byte[Constant.SampleBlockSizeInBytes];
                    while (inputStream.CanRead)
                    {
                        int bytesRead = inputStream.Read(inputBuffer, 0, inputBuffer.Length);
                        if (bytesRead < 1)
                        {
                            // workaround for NAudio bug: WaveStream.CanRead is hard coded to true regardless of position
                            break;
                        }

                        using (SampleBlock block = new SampleBlock(inputBuffer, bytesRead, SampleTypeExtensions.FromBitsPerSample(inputStream.WaveFormat.BitsPerSample)))
                        {
                            SampleBlock filteredBlock = forwardTimeFilters.Filter(block, dataPathSampleType, outputSampleType, ref recirculatingDataPathBlock);
                            outputStream.Blocks.AddLast(filteredBlock);
                        }

                        if (this.Stopping)
                        {
                            return null;
                        }
                    }
                }
                if (recirculatingDataPathBlock != null)
                {
                    recirculatingDataPathBlock.Dispose();
                }

                // release input stream as it's no longer needed and complete output
                inputStream.Dispose();
                outputStream.RecalculateBlocks();
                inputStream = outputStream;
            }

            forwardTimeFilters.Dispose();
            reverseTimeFilters.Dispose();

            performance.CompleteTimeUtc = DateTime.UtcNow;
            return inputStream;
        }

        private string GetOutputFilePath(string inputFilePath, string outputDestination, Encoding outputEncoding)
        {
            string outputExtension;
            switch (outputEncoding)
            {
                case Encoding.Flac:
                    outputExtension = Constant.Extension.Flac;
                    break;
                case Encoding.Mp3:
                    outputExtension = Constant.Extension.Mp3;
                    break;
                case Encoding.Wave:
                    outputExtension = Constant.Extension.Wave;
                    break;
                default:
                    throw new NotSupportedException(String.Format("Unhandled output file type {0}.", this.Configuration.Output.Encoding));
            }

            string inputDirectoryPath = Path.GetDirectoryName(inputFilePath);
            string outputDirectoryPath = Path.GetDirectoryName(outputDestination);
            string outputFileName;
            if (Directory.Exists(outputDestination))
            {
                // if outputDestination indicates an existing directory then take it to be the output directory path
                outputDirectoryPath = outputDestination;
                outputFileName = Path.GetFileNameWithoutExtension(inputFilePath) + outputExtension;
            }
            else if (String.IsNullOrEmpty(outputDirectoryPath))
            {
                // if outputDestination is just a file name then create the file in the input directory
                outputDirectoryPath = inputDirectoryPath;
                outputFileName = outputDestination;
            }
            else
            {
                // assume output destination is a path to a file
                // (If needed, presence of an extension could be used as a hint to determine whether it's a directory needing creation.)
                // leave outputDirectoryPath as is
                outputFileName = Path.GetFileName(outputDestination);
            }

            string outputFilePath = Path.Combine(outputDirectoryPath, outputFileName);

            // if the output file already exists and is of the same type as the input file, append the colliding file name postfix
            if (String.Equals(inputFilePath, outputFilePath, StringComparison.OrdinalIgnoreCase))
            {
                outputFilePath = Path.Combine(Path.GetDirectoryName(outputFilePath), Path.GetFileNameWithoutExtension(outputFilePath) + this.Configuration.Output.CollidingFileNamePostfix + Path.GetExtension(outputFilePath));
            }

            return outputFilePath;
        }

        private bool TryApplyMetadata(Tag inputMetadata, Tag outputMetadata)
        {
            bool metadataApplied = false;
            if (String.IsNullOrWhiteSpace(inputMetadata.Album) == false)
            {
                outputMetadata.Album = inputMetadata.Album;
                metadataApplied = true;
            }
            if (inputMetadata.AlbumArtists != null)
            {
                outputMetadata.AlbumArtists = inputMetadata.AlbumArtists;
                metadataApplied = true;
            }
            if (inputMetadata.AmazonId != null)
            {
                outputMetadata.AmazonId = inputMetadata.AmazonId;
                metadataApplied = true;
            }
            if (inputMetadata.Composers != null)
            {
                outputMetadata.Composers = inputMetadata.Composers;
                metadataApplied = true;
            }
            if (String.IsNullOrWhiteSpace(inputMetadata.Conductor) == false)
            {
                outputMetadata.Conductor = inputMetadata.Conductor;
                metadataApplied = true;
            }
            if (String.IsNullOrWhiteSpace(inputMetadata.Copyright) == false)
            {
                outputMetadata.Copyright = inputMetadata.Copyright;
                metadataApplied = true;
            }
            if (inputMetadata.Disc > 0)
            {
                outputMetadata.Disc = inputMetadata.Disc;
                metadataApplied = true;
            }
            if (inputMetadata.DiscCount > 0)
            {
                outputMetadata.DiscCount = inputMetadata.DiscCount;
                metadataApplied = true;
            }
            if (inputMetadata.Genres != null)
            {
                outputMetadata.Genres = inputMetadata.Genres;
                metadataApplied = true;
            }
            if (String.IsNullOrWhiteSpace(inputMetadata.Grouping) == false)
            {
                outputMetadata.Grouping = inputMetadata.Grouping;
                metadataApplied = true;
            }
            if (String.IsNullOrWhiteSpace(inputMetadata.Lyrics) == false)
            {
                outputMetadata.Lyrics = inputMetadata.Lyrics;
                metadataApplied = true;
            }
            if (inputMetadata.Performers != null)
            {
                outputMetadata.Performers = inputMetadata.Performers;
                metadataApplied = true;
            }
            if (inputMetadata.Pictures != null)
            {
                outputMetadata.Pictures = inputMetadata.Pictures;
                metadataApplied = true;
            }
            if (String.IsNullOrWhiteSpace(inputMetadata.Title) == false)
            {
                outputMetadata.Title = inputMetadata.Title;
                metadataApplied = true;
            }
            if (inputMetadata.Track > 0)
            {
                outputMetadata.Track = inputMetadata.Track;
                metadataApplied = true;
            }
            if (inputMetadata.TrackCount > 0)
            {
                outputMetadata.TrackCount = inputMetadata.TrackCount;
                metadataApplied = true;
            }
            if (inputMetadata.Year > 0)
            {
                outputMetadata.Year = inputMetadata.Year;
                metadataApplied = true;
            }
            return metadataApplied;
        }
    }
}
