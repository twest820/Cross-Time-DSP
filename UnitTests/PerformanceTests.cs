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
        public void FlacToInt24ViaBiquadAndFirstOrderDouble()
        {
            this.FilterFirstTrackInLibrary(SampleType.Double, SampleType.Int24, TestConstant.DefaultConfigurationFile);
        }

        [TestMethod]
        public void FlacToInt24ViaThirdOrderDouble()
        {
            this.FilterFirstTrackInLibrary(SampleType.Double, SampleType.Int24, "LinearizeDouble.xml");
        }

        [TestMethod]
        public void FlacToInt24ViaBiquadAndFirstOrderInt32()
        {
            this.FilterFirstTrackInLibrary(SampleType.Int32, SampleType.Int24, "BiquadFirstOrderQ31.xml");
        }

        private void FilterFirstTrackInLibrary(SampleType dataPathSampleType, SampleType outputSampleType, string configFilePath)
        {
            CrossTimeEngine dspEngine = new CrossTimeEngine(configFilePath, this);

            string musicFolderPath = Environment.GetFolderPath(Environment.SpecialFolder.MyMusic);
            string firstArtistPath = Directory.GetDirectories(musicFolderPath).First();
            string firstAlbumPath = Directory.GetDirectories(firstArtistPath).First();
            string firstTrackPath = Directory.GetFiles(firstAlbumPath, "*.flac").First();

            this.TestContext.WriteLine("Load and buffer");
            DateTime loadStartedUtc = DateTime.UtcNow;
            MediaFoundationReader inputStream = new MediaFoundationReader(firstTrackPath);
            using (SampleBuffer inputBuffer = new SampleBuffer(inputStream))
            {
                DateTime loadStoppedUtc = DateTime.UtcNow;
                this.TestContext.WriteLine("{0}", (loadStoppedUtc - loadStartedUtc).ToString(Constant.ElapsedTimeFormat));

                this.TestContext.WriteLine("Warmup iterations");
                for (int warmup = 0; warmup < 10; ++warmup)
                {
                    this.FilterStream(dspEngine.Configuration, inputBuffer, dataPathSampleType, outputSampleType);
                }

                this.TestContext.WriteLine("Running iterations");
                for (int iteration = 0; iteration < 25; ++iteration)
                {
                    this.FilterStream(dspEngine.Configuration, inputBuffer, dataPathSampleType, outputSampleType);
                }
            }
        }

        private void FilterStream(CrossTimeDspConfiguration configuration, SampleBuffer inputBuffer, SampleType dataPathSampleType, SampleType outputSampleType)
        {
            // setup
            FilterBank filters = new FilterBank(configuration.Engine.Precision, inputBuffer.WaveFormat.SampleRate, inputBuffer.WaveFormat.Channels, configuration.Engine.Q31Adaptive.Q31_32x64_Threshold, configuration.Engine.Q31Adaptive.Q31_64x64_Threshold);
            foreach (Filter filter in configuration.Filters)
            {
                filter.AddTo(filters);
            }

            // time duration of reverse time pass
            DateTime filteringStartedUtc = DateTime.UtcNow;
            using (SampleBuffer reverseTimeBuffer = new SampleBuffer(inputBuffer.WaveFormat.SampleRate, outputSampleType.BitsPerSample(), inputBuffer.WaveFormat.Channels))
            {
                SampleBlock recirculatingDataPathBlock = null;
                for (LinkedListNode<SampleBlock> blockNode = inputBuffer.Blocks.Last; blockNode != null; blockNode = blockNode.Previous)
                {
                    SampleBlock filteredBlock = filters.FilterReverse(blockNode.Value, dataPathSampleType, outputSampleType, ref recirculatingDataPathBlock);
                    reverseTimeBuffer.Blocks.AddFirst(filteredBlock);
                }
                if (recirculatingDataPathBlock != null)
                {
                    recirculatingDataPathBlock.Dispose();
                }
                reverseTimeBuffer.RecalculateBlocks();

                DateTime filteringStoppedUtc = DateTime.UtcNow;
                if (filters.TimingAvailable)
                {
                    this.TestContext.WriteLine("{0} ({1} {2} {3})", (filteringStoppedUtc - filteringStartedUtc).ToString(Constant.ElapsedTimeFormat), filters.ToDataPathTime.ToString(Constant.ElapsedTimeFormat), filters.FilterTime.ToString(Constant.ElapsedTimeFormat), filters.ToOutputTime.ToString(Constant.ElapsedTimeFormat));
                }
                else
                {
                    this.TestContext.WriteLine("{0}", (filteringStoppedUtc - filteringStartedUtc).ToString(Constant.ElapsedTimeFormat));
                }
            }
        }
    }
}
