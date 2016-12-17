using CrossTimeDsp.Configuration;
using CrossTimeDsp.Dsp;
using CrossTimeDsp.Encodings;
using CrossTimeDsp.Encodings.Flac;
using CrossTimeDsp.Encodings.MP3;
using CrossTimeDsp.Encodings.Wav;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Reflection;
using System.Threading;
using System.Threading.Tasks;

namespace CrossTimeDsp
{
    public class CrossTimeEngine
    {
        private const double ClipWarningThreshold = 0.01;
        private static readonly TimeSpan FileDeletionRetryInterval = TimeSpan.FromSeconds(1);

        private DateTime configurationLastWriteTimeUtc;

        public CrossTimeDspSection Configuration { get; private set; }

        public CrossTimeEngine()
        {
            this.Configuration = CrossTimeDspSection.Load();
            // .NET configuration has many possible configuration files where Cross Time DSP settings could potentially be specified (user 
            // config, hosting process config, dll config, machine config...), the majority of which are rarely used
            // instead of trying to probe all possible configuration sources, which will most likely miss some and possibly pull in edits
            // not relevant to Cross Time DSP, it is required to edit Cross Time DSP's app.config directly to trigger an update of existing
            // output files---users using alternate configuration mechanisms will need to delete the output files manually
            string appConfigPath = String.Format("{0}.config", Assembly.GetExecutingAssembly().Location);
            this.configurationLastWriteTimeUtc = File.GetLastWriteTimeUtc(appConfigPath);
        }

        public void FilterFile(string inputFilePath)
        {
            this.FilterFile(inputFilePath, this.GetOutputFilePathInSameDirectory(inputFilePath, this.Configuration.Output.Encoding));
        }

        public void FilterFile(string inputFilePath, string outputFilePath)
        {
            // nothing to do if the output file already exists and is newer than both the input file and Cross Time DSP's app.config file
            if (File.Exists(outputFilePath))
            {
                DateTime inputLastWriteTimeUtc = File.GetLastWriteTimeUtc(inputFilePath);
                DateTime outputLastWriteTimeUtc = File.GetLastWriteTimeUtc(outputFilePath);
                if (outputLastWriteTimeUtc > inputLastWriteTimeUtc)
                {
                    Trace.TraceInformation("Skipping '{0}' as '{1}' is newer.", Path.GetFileName(inputFilePath), Path.GetFileName(outputFilePath));
                    return;
                }
            }

            DateTime processingStart = DateTime.UtcNow;
            Trace.TraceInformation("Processing '{0}' to '{1}'...", Path.GetFileName(inputFilePath), Path.GetFileName(outputFilePath));

            // if input file is not wav, convert it to wav
            string inputWavFilePath;
            Dictionary<MetadataType, string> inputMetadata = null;
            switch (Path.GetExtension(inputFilePath))
            {
                case FlacConstants.FileExtension:
                    inputWavFilePath = Path.GetTempFileName();
                    FlacConverter flacConverter = new FlacConverter(this.Configuration.Output.Flac.FlacPath);
                    inputMetadata = flacConverter.ConvertFlacToWav(inputFilePath, inputWavFilePath);
                    break;
                case MP3Constants.FileExtension:
                    inputWavFilePath = Path.GetTempFileName();
                    MP3Converter mp3Converter = new MP3Converter(this.Configuration.Output.MP3.LamePath);
                    mp3Converter.ConvertMP3ToWav(inputFilePath, inputWavFilePath);
                    break;
                case WavConstants.FileExtension:
                    inputWavFilePath = inputFilePath;
                    break;
                default:
                    throw new NotSupportedException(String.Format("Unknown file type associated with the extension of '{0}'.", inputFilePath));
            }

            // load input file
            WavStream inputFile = new WavStream(inputWavFilePath);

            // ensure output directory exists so that output file write succeeds
            // this may not be needed right away if an intermediate .wav file is generated, but it'll be needed sooner or later and it's
            // simplest to do the generation before output begins
            string outputDirectoryPath = Path.GetDirectoryName(outputFilePath);
            if (String.IsNullOrEmpty(outputDirectoryPath) == false && Directory.Exists(outputDirectoryPath) == false)
            {
                Directory.CreateDirectory(outputDirectoryPath);
            }

            // create output file
            // file name to write to depends on whether the .wav is an intermediate .wav which will be transcoded to another format or if
            // the .wav is the final output file
            string outputWavFilePath = outputFilePath;
            if (this.Configuration.Output.Encoding != Encoding.Wav)
            {
                outputWavFilePath = Path.GetTempFileName();
            }
            WavStream outputFile = new WavStream(outputWavFilePath, inputFile, this.Configuration.Output.Wav.BitsPerSample);

            // actually do the filtering
            DateTime filteringStart = DateTime.UtcNow;
            switch (this.Configuration.Engine.Precision)
            {
                case FilterPrecision.Double:
                    this.FilterStream<double>(inputFile, outputFile, inputFile.TotalSamples);
                    break;
                case FilterPrecision.Q31:
                case FilterPrecision.Q31_32x64:
                case FilterPrecision.Q31_64x64:
                case FilterPrecision.Q31Adaptive:
                    this.FilterStream<Int32>(inputFile, outputFile, inputFile.TotalSamples);
                    break;
                default:
                    throw new NotSupportedException(String.Format("Unsupported filter precision {0}.", this.Configuration.Engine.Precision));
            }

            // close streams
            inputFile.Close();
            outputFile.Close();

            // if the requested output file type is other than .wav, transcode to the desired output file format
            DateTime conversionStart = DateTime.UtcNow;
            Dictionary<MetadataType, string> outputMetadata = inputMetadata;
            if (outputMetadata == null)
            {
                outputMetadata = outputFile.GetMetadata();
            }
            if (this.Configuration.Output.Encoding != Encoding.Wav)
            {
                switch (this.Configuration.Output.Encoding)
                {
                    case Encoding.Flac:
                        FlacConverter flacConverter = new FlacConverter(this.Configuration.Output.Flac.FlacPath);
                        flacConverter.OutputBitsPerSample = outputFile.SampleSizeInBits;
                        flacConverter.CompressionLevel = this.Configuration.Output.Flac.CompressionLevel;
                        flacConverter.ConvertWavToFlac(outputWavFilePath, outputFilePath, outputMetadata);
                        break;
                    default:
                        throw new NotSupportedException(String.Format("Unhandled output encoding {0}.", this.Configuration.Output.Encoding));
                }
                // don't need the intermediate .wav files any more, so remove it to save disk space
                // file system operations aren't entirely synchronous, so back off and retry and then fail if the retry also fails
                if (inputFilePath != inputWavFilePath)
                {
                    try
                    {
                        File.Delete(inputWavFilePath);
                    }
                    catch (IOException)
                    {
                        Thread.Sleep(CrossTimeEngine.FileDeletionRetryInterval);
                        File.Delete(inputWavFilePath);
                    }
                }
                try
                {
                    File.Delete(outputWavFilePath);
                }
                catch (IOException)
                {
                    Thread.Sleep(CrossTimeEngine.FileDeletionRetryInterval);
                    File.Delete(outputWavFilePath);
                }
            }

            Console.WriteLine("Processed {0} in {1} (input {2}, filter {3}, output {4}).", Path.GetFileName(inputFilePath), (DateTime.UtcNow - processingStart).ToString(@"mm\:ss\.fff"), (filteringStart - processingStart).ToString(@"mm\:ss\.fff"), (processingStart - conversionStart).ToString(@"mm\:ss\.fff"), (DateTime.UtcNow - conversionStart).ToString(@"mm\:ss\.fff"));
        }

        public void FilterFiles(string inputDirectoryPath, string searchPattern, string outputPath)
        {
            // trace directory which was just entered
            // this makes it easier to monitor progress when processing large numbers of artists and albums
            Trace.TraceInformation("Processing '{0}'...", inputDirectoryPath);

            // process files in this directory matching the search pattern
            ParallelOptions parallelOptions = new ParallelOptions();
            parallelOptions.MaxDegreeOfParallelism = Environment.ProcessorCount;
            Parallel.ForEach(Directory.GetFiles(inputDirectoryPath, searchPattern), parallelOptions, (string inputFilePath) =>
            {
                // if the file was obviously created by Cross Time DSP processing another file, skip it
                if (String.IsNullOrEmpty(this.Configuration.Output.CollidingFileNamePostfix) == false && Path.GetFileNameWithoutExtension(inputFilePath).EndsWith(this.Configuration.Output.CollidingFileNamePostfix))
                {
                    return;
                }
                // otherwise, process this input file
                string outputFilePath;
                if (String.Equals(inputDirectoryPath, outputPath, StringComparison.OrdinalIgnoreCase))
                {
                    outputFilePath = this.GetOutputFilePathInSameDirectory(inputFilePath, this.Configuration.Output.Encoding);
                }
                else
                {
                    outputFilePath = this.GetOutputFilePathInDifferentDirectory(inputFilePath, outputPath, this.Configuration.Output.Encoding);
                }
                this.FilterFile(inputFilePath, outputFilePath);
            });

            // recurse through subdirectories
            foreach (string inputSubdirectoryPath in Directory.GetDirectories(inputDirectoryPath))
            {
                string inputSubdirectoryName = Path.GetFileName(inputSubdirectoryPath);
                this.FilterFiles(inputSubdirectoryPath, searchPattern, Path.Combine(outputPath, inputSubdirectoryName));
            }
        }

        private void FilterStream<TSample>(SampledStream inputStream, SampledStream outputStream, Int64 totalSamples) where TSample : struct
        {
            // populate filters
            FilterBank<TSample> forwardTimeFilters = new FilterBank<TSample>(inputStream.Channels);
            FilterBank<TSample> reverseTimeFilters = new FilterBank<TSample>(inputStream.Channels);

            double antiClippingGain = Math.Pow(10.0, this.Configuration.Engine.ReverseTimeAntiClippingAttenuationInDB / 20.0);
            reverseTimeFilters.Add(Gain.Create<TSample>(antiClippingGain, this.Configuration.Engine.Precision));

            foreach (FilterElement filter in this.Configuration.Filters.Filters)
            {
                switch (filter.TimeDirection)
                {
                    case TimeDirection.Forward:
                        forwardTimeFilters.Add(filter.Create<TSample>(inputStream.SamplesPerSecond, this.Configuration.Engine));
                        break;
                    case TimeDirection.Reverse:
                        reverseTimeFilters.Add(filter.Create<TSample>(inputStream.SamplesPerSecond, this.Configuration.Engine));
                        break;
                    default:
                        throw new NotSupportedException(String.Format("Unhandled time direction {0}.", filter.TimeDirection));
                }
            }

            // do filtering
            IEnumerable<TSample> samples;
            if (typeof(TSample) == typeof(double))
            {
                samples = (IEnumerable<TSample>)new SampledStreamConverter<double>(inputStream);
            }
            else if (typeof(TSample) == typeof(Int32))
            {
                samples = (IEnumerable<TSample>)inputStream;
            }
            else
            {
                throw new NotSupportedException(String.Format("Unhandled sample type {0}.", typeof(TSample).FullName));
            }

            // do reverse time pass
            // if the only reverse time filter is the anti-clipping gain then there's nothing to do
            if (reverseTimeFilters.FilterCount > inputStream.Channels)
            {
                ReversedTimeBuffer<TSample> sampleBuffer = new ReversedTimeBuffer<TSample>(samples, totalSamples);
                for (Int64 index = sampleBuffer.TotalSamples - 1; index >= 0; --index)
                {
                    sampleBuffer[index] = reverseTimeFilters.Filter(sampleBuffer[index]);
                }
                // point forward time pass at output of reverse time pass
                samples = sampleBuffer;
            }

            // do forward time pass
            if (forwardTimeFilters.FilterCount > 0)
            {
                samples = forwardTimeFilters.Filter(samples);
            }

            // write results to output
            ISampleConverter<TSample> sampleConverter;
            if (typeof(TSample) == typeof(double))
            {
                sampleConverter = (ISampleConverter<TSample>)new DoubleSampleConverter();
            }
            else if (typeof(TSample) == typeof(Int32))
            {
                sampleConverter = (ISampleConverter<TSample>)new Int32SampleConverter();
            }
            else
            {
                throw new NotSupportedException(String.Format("Unhandled sample type {0}.", typeof(TSample).FullName));
            }
            foreach (TSample sample in samples)
            {
                Int32 sampleAsQ31 = sampleConverter.SampleToQ31(sample);
                outputStream.WriteSample(sampleAsQ31);
            }

            // force, er, strongly encourage garbage collection of reverse time buffer
            // if this isn't done it's likely processing large tracks consecutively will fail as sampleBuffer is a long lived object and
            // likely gets promoted an older generation, meaning FilterFile() can get invoked again before sampleBuffer gets collected
            samples = null;
            GC.Collect();
        }

        private string GetOutputFilePathInDifferentDirectory(string inputFilePath, string outputDirectoryPath, Encoding outputEncoding)
        {
            string outputExtension;
            switch (outputEncoding)
            {
                case Encoding.Flac:
                    outputExtension = FlacConstants.FileExtension;
                    break;
                case Encoding.Wav:
                    outputExtension = WavConstants.FileExtension;
                    break;
                default:
                    throw new NotSupportedException(String.Format("Unhandled output file type {0}.", this.Configuration.Output.Encoding));
            }
            return String.Format("{0}{1}{2}{3}", outputDirectoryPath, Path.DirectorySeparatorChar, Path.GetFileNameWithoutExtension(inputFilePath), outputExtension);
        }

        private string GetOutputFilePathInSameDirectory(string inputFilePath, Encoding outputEncoding)
        {
            // get the default output file name
            string directoryPath = Path.GetDirectoryName(inputFilePath);
            string outputFilePath = this.GetOutputFilePathInDifferentDirectory(inputFilePath, directoryPath, outputEncoding);

            // if the output file already exists and is of the same type as the input file, append the colliding file name postfix
            if (File.Exists(outputFilePath))
            {
                string inputExtension = Path.GetExtension(inputFilePath);
                string outputExtension = Path.GetExtension(outputFilePath);
                if (String.Equals(inputExtension, outputExtension, StringComparison.OrdinalIgnoreCase))
                {
                    outputFilePath = String.Format("{0}{1}{2}{3}{4}", directoryPath, Path.DirectorySeparatorChar, Path.GetFileNameWithoutExtension(inputFilePath), this.Configuration.Output.CollidingFileNamePostfix, outputExtension);
                }
            }
            return outputFilePath;
        }
    }
}
