using CrossTimeDsp.Configuration;
using CrossTimeDsp.Dsp;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using NAudio.MediaFoundation;
using NAudio.Wave;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;

namespace CrossTimeDsp.UnitTests
{
    [TestClass]
    public class PerformanceTests : CrossTimeTest
    {
        [ClassInitialize]
        public static void ClassInitialize(TestContext testContext)
        {
            MediaFoundationApi.Startup();
        }

        [ClassCleanup]
        public static void ClassCleanup()
        {
            MediaFoundationApi.Shutdown();
        }

        [TestMethod]
        public void FlacToInt24OutputViaDouble()
        {
            this.FilterFirstTrackInLibrary(SampleType.Double, SampleType.Int24);
        }

        [TestMethod]
        public void FlacToInt24OutputViaInt32()
        {
            this.FilterFirstTrackInLibrary(SampleType.Int32, SampleType.Int24);
        }

        private void FilterFirstTrackInLibrary(SampleType dataPathSampleType, SampleType outputSampleType)
        {
            CrossTimeEngine dspEngine = new CrossTimeEngine(TestConstant.ConfigurationFile, this);

            string musicFolderPath = Environment.GetFolderPath(Environment.SpecialFolder.MyMusic);
            string firstArtistPath = Directory.GetDirectories(musicFolderPath).First();
            string firstAlbumPath = Directory.GetDirectories(firstArtistPath).First();
            string firstTrackPath = Directory.GetFiles(firstAlbumPath, "*.flac").First();

            this.TestContext.WriteLine("Load and buffer");
            DateTime loadStartedUtc = DateTime.UtcNow;
            MediaFoundationReader inputStream = new MediaFoundationReader(firstTrackPath);
            SampleBuffer inputBuffer = new SampleBuffer(inputStream);
            DateTime loadStoppedUtc = DateTime.UtcNow;
            this.TestContext.WriteLine("{0}", (loadStoppedUtc - loadStartedUtc).ToString(Constant.ElapsedTimeFormat));

            this.TestContext.WriteLine("Warmup iterations");
            for (int warmup = 0; warmup < 2; ++warmup)
            {
                this.FilterStream(dspEngine.Configuration, inputBuffer, dataPathSampleType, outputSampleType);
            }
            this.TestContext.WriteLine("Running iterations");
            for (int iteration = 0; iteration < 25; ++iteration)
            {
                this.FilterStream(dspEngine.Configuration, inputBuffer, dataPathSampleType, outputSampleType);
            }
        }

        private void FilterStream(CrossTimeDspConfiguration configuration, SampleBuffer inputBuffer, SampleType dataPathSampleType, SampleType outputSampleType)
        {
            bool doubleDataPath = dataPathSampleType == SampleType.Double;
            if (doubleDataPath == false)
            {
                configuration.Engine.Precision = FilterPrecision.Q31Adaptive;
            }
            FilterBank filters = new FilterBank();
            foreach (FilterElement filter in configuration.Filters.Filters)
            {
                if (doubleDataPath)
                {
                    filters.Add(filter.Create<double>(inputBuffer.WaveFormat.SampleRate, configuration.Engine, inputBuffer.WaveFormat.Channels));
                }
                else
                {
                    filters.Add(filter.Create<int>(inputBuffer.WaveFormat.SampleRate, configuration.Engine, inputBuffer.WaveFormat.Channels));
                }
            }

            DateTime filteringStartedUtc = DateTime.UtcNow;
            SampleBuffer reverseTimeBuffer = new SampleBuffer(inputBuffer.WaveFormat.SampleRate, outputSampleType.BitsPerSample(), inputBuffer.WaveFormat.Channels);
            for (LinkedListNode<SampleBlock> blockNode = inputBuffer.Blocks.Last; blockNode != null; blockNode = blockNode.Previous)
            {
                SampleBlock block = blockNode.Value;
                SampleBlock filteredBlock = filters.FilterReverse(block, dataPathSampleType, outputSampleType);
                reverseTimeBuffer.Blocks.AddFirst(filteredBlock);
            }
            reverseTimeBuffer.RecalculateBlocks();

            DateTime filteringStoppedUtc = DateTime.UtcNow;
            this.TestContext.WriteLine("{0}", (filteringStoppedUtc - filteringStartedUtc).ToString(Constant.ElapsedTimeFormat));
        }
    }
}
