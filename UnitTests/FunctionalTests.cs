using CrossTimeDsp.Configuration;
using CrossTimeDsp.Dsp;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using NAudio.Wave;
using System;
using System.Diagnostics;
using System.IO;
using TagLib;
using File = System.IO.File;
using TagFile = TagLib.File;

namespace CrossTimeDsp.UnitTests
{
    [TestClass]
    public class FunctionalTests : CrossTimeTest
    {
        private BiquadElement CreateBiquad(FilterType type, double f0, double gainInDB, double q, TimeDirection timeDirection)
        {
            BiquadElement biquad = new BiquadElement();
            biquad.F0 = f0;
            biquad.GainInDB = gainInDB;
            biquad.Q = q;
            biquad.TimeDirection = timeDirection;
            biquad.Type = type;
            return biquad;
        }

        [TestMethod]
        public void FileCreationAndPreservation()
        {
            string inputFileName = "fileCreationAndPreservation.wav";
            string inputFilePath = Path.Combine(Environment.CurrentDirectory, inputFileName);
            if (!File.Exists(inputFilePath))
            {
                File.Copy(TestConstant.SourceFilePath16Bit, inputFilePath);
            }

            CrossTimeEngine engine = new CrossTimeEngine(TestConstant.ConfigurationFile, this);
            string outputFileName = String.Format("{0}{1}.wav", Path.GetFileNameWithoutExtension(inputFilePath), engine.Configuration.Output.CollidingFileNamePostfix);
            this.RemoveExistingOutputFiles(outputFileName);
            DateTime startUtc = DateTime.UtcNow;

            // process file and output file should be created
            engine.Configuration.Filters.Filters.Clear();
            engine.FilterFiles(Environment.CurrentDirectory, inputFileName, Environment.CurrentDirectory);
            DateTime firstOutputFileWriteTimeUtc = File.GetLastWriteTimeUtc(outputFileName);
            Assert.IsTrue(firstOutputFileWriteTimeUtc > startUtc);

            // since no filters are configured the output file should be a clone of the input
            this.VerifyWaveFilesEquivalent(outputFileName, inputFileName, 1.0, 0.0, true);

            // filter again and the output file should not be touched
            engine.FilterFiles(Environment.CurrentDirectory, inputFileName, Environment.CurrentDirectory);
            DateTime secondOutputFileWriteTimeUtc = File.GetLastWriteTimeUtc(outputFileName);
            Assert.AreEqual(firstOutputFileWriteTimeUtc, secondOutputFileWriteTimeUtc, "Output file {0} was touched after initial write at {1} by second write at {2}.", inputFilePath, firstOutputFileWriteTimeUtc, secondOutputFileWriteTimeUtc);
        }

        [TestMethod]
        public void ForwardTimeCrossoverAndEQ()
        {
            // setup
            string midrangeFileNameDouble = "crossoverAndEQ.midrange.44100.24.double.wav";
            string midrangeFileNameQ31 = "crossoverAndEQ.midrange.44100.24.Q31.wav";
            string midrangeFileNameQ31_32x64 = "crossoverAndEQ.midrange.44100.24.Q31_32x64.wav";
            string midrangeFileNameQ31Adaptive = "crossoverAndEQ.midrange.44100.24.Q31Adaptive.wav";
            string tweeterFileNameDouble = "crossoverAndEQ.tweeter.44100.24.double.wav";
            string tweeterFileNameQ31 = "crossoverAndEQ.tweeter.44100.24.Q31.wav";
            string tweeterFileNameQ31_32x64 = "crossoverAndEQ.tweeter.44100.24.Q31_32x64.wav";
            string tweeterFileNameQ31Adaptive = "crossoverAndEQ.tweeter.44100.24.Q31.wav";
            string subwooferFileNameDouble = "crossoverAndEQ.subwoofer.44100.24.double.wav";
            string subwooferFileNameQ31_32x64 = "crossoverAndEQ.subwoofer.44100.24.Q31_32x64.wav";
            string subwooferFileNameQ31_64x64 = "crossoverAndEQ.subwoofer.44100.24.Q31_64x64.wav";
            string subwooferFileNameQ31Adaptive = "crossoverAndEQ.subwoofer.44100.24.Q31.wav";
            string subwooferLinearizedFileNameDouble = "crossoverEQAndLinearization.subwoofer.44100.24.double.wav";
            string subwooferLinearizedFileNameQ31Adaptive = "crossoverEQAndLinearization.subwoofer.44100.24.Q31.wav";
            this.RemoveExistingOutputFiles(midrangeFileNameDouble, midrangeFileNameQ31, midrangeFileNameQ31_32x64, midrangeFileNameQ31Adaptive,
                                           tweeterFileNameDouble, tweeterFileNameQ31, tweeterFileNameQ31_32x64, tweeterFileNameQ31Adaptive,
                                           subwooferFileNameDouble, subwooferFileNameQ31_32x64, subwooferFileNameQ31_64x64, subwooferFileNameQ31Adaptive,
                                           subwooferLinearizedFileNameDouble, subwooferLinearizedFileNameQ31Adaptive);
            CrossTimeEngine engine = new CrossTimeEngine(TestConstant.ConfigurationFile, this);

            // midrange
            engine.Configuration.Filters.Filters.Clear();
            // 200Hz LR6 highpass
            engine.Configuration.Filters.Filters.Add(this.CreateBiquad(FilterType.Highpass, 200.0, 0.0, 0.5, TimeDirection.Forward));
            engine.Configuration.Filters.Filters.Add(this.CreateBiquad(FilterType.Highpass, 200.0, 0.0, 1.0, TimeDirection.Forward));
            engine.Configuration.Filters.Filters.Add(this.CreateBiquad(FilterType.Highpass, 200.0, 0.0, 1.0, TimeDirection.Forward));
            // 1700Hz LR6 lowpass
            engine.Configuration.Filters.Filters.Add(this.CreateBiquad(FilterType.Lowpass, 1700.0, 0.0, 0.5, TimeDirection.Forward));
            engine.Configuration.Filters.Filters.Add(this.CreateBiquad(FilterType.Lowpass, 1700.0, 0.0, 1.0, TimeDirection.Forward));
            engine.Configuration.Filters.Filters.Add(this.CreateBiquad(FilterType.Lowpass, 1700.0, 0.0, 1.0, TimeDirection.Forward));
            // dipole EQ
            engine.Configuration.Filters.Filters.Add(this.CreateBiquad(FilterType.Peaking, 150.0, 15.0, 0.5, TimeDirection.Forward));
            // driver EQ
            engine.Configuration.Filters.Filters.Add(this.CreateBiquad(FilterType.Peaking, 550.0, 4.0, 1.0, TimeDirection.Forward));

            engine.Configuration.Engine.Precision = FilterPrecision.Double;
            engine.FilterFiles(Environment.CurrentDirectory, TestConstant.SourceFilePath16Bit, midrangeFileNameDouble);
            engine.Configuration.Engine.Precision = FilterPrecision.Q31;
            engine.FilterFiles(Environment.CurrentDirectory, TestConstant.SourceFilePath16Bit, midrangeFileNameQ31);
            this.VerifyWaveFilesEquivalent(midrangeFileNameQ31, midrangeFileNameDouble, 1.0, 56.0 * TestConstant.Q23ToQ31TruncationErrorIncrease, true);
            engine.Configuration.Engine.Precision = FilterPrecision.Q31_32x64;
            engine.FilterFiles(Environment.CurrentDirectory, TestConstant.SourceFilePath16Bit, midrangeFileNameQ31_32x64);
            this.VerifyWaveFilesEquivalent(midrangeFileNameQ31_32x64, midrangeFileNameDouble, 1.0, 6.0 * TestConstant.Q23ToQ31TruncationErrorIncrease, true);
            engine.Configuration.Engine.Precision = FilterPrecision.Q31Adaptive;
            engine.FilterFiles(Environment.CurrentDirectory, TestConstant.SourceFilePath16Bit, midrangeFileNameQ31Adaptive);
            this.VerifyWaveFilesEquivalent(midrangeFileNameQ31Adaptive, midrangeFileNameDouble, 1.0, 6.0 * TestConstant.Q23ToQ31TruncationErrorIncrease, true);

            // tweeter
            engine.Configuration.Filters.Filters.Clear();
            // 1700Hz LR6 highpass
            engine.Configuration.Filters.Filters.Add(this.CreateBiquad(FilterType.Highpass, 1700.0, 0.0, 0.5, TimeDirection.Forward));
            engine.Configuration.Filters.Filters.Add(this.CreateBiquad(FilterType.Highpass, 1700.0, 0.0, 1.0, TimeDirection.Forward));
            engine.Configuration.Filters.Filters.Add(this.CreateBiquad(FilterType.Highpass, 1700.0, 0.0, 1.0, TimeDirection.Forward));
            // dipole EQ
            engine.Configuration.Filters.Filters.Add(this.CreateBiquad(FilterType.Peaking, 1400.0, 5.0, 1.9, TimeDirection.Forward));
            engine.Configuration.Filters.Filters.Add(this.CreateBiquad(FilterType.Peaking, 3800.0, -2.0, 3.0, TimeDirection.Forward));
            engine.Configuration.Filters.Filters.Add(this.CreateBiquad(FilterType.Peaking, 7300.0, 3.5, 2.0, TimeDirection.Forward));
            engine.Configuration.Filters.Filters.Add(this.CreateBiquad(FilterType.Peaking, 13000.0, -3.5, 4.0, TimeDirection.Forward));

            Trace.TraceInformation(String.Empty);
            engine.Configuration.Engine.Precision = FilterPrecision.Double;
            engine.FilterFiles(Environment.CurrentDirectory, TestConstant.SourceFilePath16Bit, tweeterFileNameDouble);
            engine.Configuration.Engine.Precision = FilterPrecision.Q31;
            engine.FilterFiles(Environment.CurrentDirectory, TestConstant.SourceFilePath16Bit, tweeterFileNameQ31);
            this.VerifyWaveFilesEquivalent(tweeterFileNameQ31, tweeterFileNameDouble, 1.0, 1.0 * TestConstant.Q23ToQ31TruncationErrorIncrease, true);
            engine.Configuration.Engine.Precision = FilterPrecision.Q31_32x64;
            engine.FilterFiles(Environment.CurrentDirectory, TestConstant.SourceFilePath16Bit, tweeterFileNameQ31_32x64);
            this.VerifyWaveFilesEquivalent(tweeterFileNameQ31_32x64, tweeterFileNameDouble, 1.0, 1.0 * TestConstant.Q23ToQ31TruncationErrorIncrease, true);
            engine.Configuration.Engine.Precision = FilterPrecision.Q31Adaptive;
            engine.FilterFiles(Environment.CurrentDirectory, TestConstant.SourceFilePath16Bit, tweeterFileNameQ31Adaptive);
            this.VerifyWaveFilesEquivalent(tweeterFileNameQ31Adaptive, tweeterFileNameDouble, 1.0, 1.0 * TestConstant.Q23ToQ31TruncationErrorIncrease, true);

            // subwoofer
            engine.Configuration.Filters.Filters.Clear();
            // 200Hz LR6 lowpass
            engine.Configuration.Filters.Filters.Add(this.CreateBiquad(FilterType.Lowpass, 200.0, 0.0, 0.5, TimeDirection.Forward));
            engine.Configuration.Filters.Filters.Add(this.CreateBiquad(FilterType.Lowpass, 200.0, 0.0, 1.0, TimeDirection.Forward));
            engine.Configuration.Filters.Filters.Add(this.CreateBiquad(FilterType.Lowpass, 200.0, 0.0, 1.0, TimeDirection.Forward));
            // dipole EQ
            engine.Configuration.Filters.Filters.Add(this.CreateBiquad(FilterType.Peaking, 39.0, 14.0, 0.5, TimeDirection.Forward));
            engine.Configuration.Filters.Filters.Add(this.CreateBiquad(FilterType.Peaking, 180.0, 2.0, 1.0, TimeDirection.Forward));
            // forward time phase linearization
            engine.Configuration.Filters.Filters.Add(this.CreateBiquad(FilterType.Allpass, 150.0, 0.0, 0.5 * Math.Sqrt(2.0), TimeDirection.Forward));

            Trace.TraceInformation(String.Empty);
            engine.Configuration.Engine.Precision = FilterPrecision.Double;
            engine.FilterFiles(Environment.CurrentDirectory, TestConstant.SourceFilePath16Bit, subwooferFileNameDouble);
            engine.Configuration.Engine.Precision = FilterPrecision.Q31_32x64;
            engine.FilterFiles(Environment.CurrentDirectory, TestConstant.SourceFilePath16Bit, subwooferFileNameQ31_32x64);
            this.VerifyWaveFilesEquivalent(subwooferFileNameQ31_32x64, subwooferFileNameDouble, 1.0, 37.0 * TestConstant.Q23ToQ31TruncationErrorIncrease, true);
            engine.Configuration.Engine.Precision = FilterPrecision.Q31_64x64;
            engine.FilterFiles(Environment.CurrentDirectory, TestConstant.SourceFilePath16Bit, subwooferFileNameQ31_64x64);
            this.VerifyWaveFilesEquivalent(subwooferFileNameQ31_64x64, subwooferFileNameDouble, 1.0, 1.0 * TestConstant.Q23ToQ31TruncationErrorIncrease, true);
            engine.Configuration.Engine.Precision = FilterPrecision.Q31Adaptive;
            engine.FilterFiles(Environment.CurrentDirectory, TestConstant.SourceFilePath16Bit, subwooferFileNameQ31Adaptive);
            this.VerifyWaveFilesEquivalent(subwooferFileNameQ31Adaptive, subwooferFileNameDouble, 1.0, 28.0 * TestConstant.Q23ToQ31TruncationErrorIncrease, true);

            // subwoofer with reverse time phase linearization
            engine.Configuration.Filters.Filters.Add(this.CreateBiquad(FilterType.Allpass, 200.0, 0.0, 1.0, TimeDirection.Reverse));
            engine.Configuration.Filters.Filters.Add(this.CreateBiquad(FilterType.Allpass, 39.0, 0.0, 0.5 * Math.Sqrt(2.0), TimeDirection.Reverse));

            engine.Configuration.Engine.Precision = FilterPrecision.Double;
            engine.FilterFiles(Environment.CurrentDirectory, TestConstant.SourceFilePath16Bit, subwooferLinearizedFileNameDouble);
            engine.Configuration.Engine.Precision = FilterPrecision.Q31Adaptive;
            engine.FilterFiles(Environment.CurrentDirectory, TestConstant.SourceFilePath16Bit, subwooferLinearizedFileNameQ31Adaptive);
            this.VerifyWaveFilesEquivalent(subwooferLinearizedFileNameQ31Adaptive, subwooferLinearizedFileNameDouble, 1.0, 28.0 * TestConstant.Q23ToQ31TruncationErrorIncrease, true);
        }

        [TestMethod]
        public void InverseAllpassAndWav()
        {
            // remove any existing output files to guarantee test runs on up to date data
            string outputFileName16Bit = "inverseAllpassAndWav.44100.16.double.wav";
            string outputFileName24Bit = "inverseAllpassAndWav.44100.24.double.wav";
            string outputFileNameQ31x63 = "inverseAllpassAndWav.44100.24.Q31_32x64.wav";
            this.RemoveExistingOutputFiles(outputFileName16Bit, outputFileName24Bit, outputFileNameQ31x63);

            // run test cases
            // check 24 bit cases
            // scale factor is quirky as the initial 24 bit reference file was created with a slightly different approach towards
            // anti-clipping attenuation
            CrossTimeEngine engine = new CrossTimeEngine(TestConstant.ConfigurationFile, this);
            engine.FilterFiles(Environment.CurrentDirectory, TestConstant.SourceFilePath16Bit, outputFileName24Bit);
            string referenceFilePath24Bit = "whiteNoise.InverseAllpass.44100.24.wav";
            double referenceFileScaleFactor = 1.019154;
            this.VerifyWaveFilesEquivalent(outputFileName24Bit, referenceFilePath24Bit, referenceFileScaleFactor, 1.5 * TestConstant.Q23ToQ31TruncationErrorIncrease, true);

            // check cross bitness case - cases in the same bitness are less likely to catch conversion and 24 bit handling defects
            // tolerance here may seem loose but comparing 24 bit samples to their truncated 16 bit form is inexact
            string referenceFilePath16Bit = "whiteNoise.InverseAllpass.44100.16.wav";
            this.VerifyWaveFilesEquivalent(outputFileName24Bit, referenceFilePath16Bit, 0.3162 * 256.0, 40.5 * TestConstant.Q23ToQ31TruncationErrorIncrease, false);

            // check fixed point
            engine.Configuration.Engine.Precision = FilterPrecision.Q31_32x64;
            engine.FilterFiles(Environment.CurrentDirectory, TestConstant.SourceFilePath16Bit, outputFileNameQ31x63);
            this.VerifyWaveFilesEquivalent(outputFileNameQ31x63, referenceFilePath24Bit, referenceFileScaleFactor, 1.6 * TestConstant.Q23ToQ31TruncationErrorIncrease, false);

            // check 16 bit cases
            engine.Configuration.Engine.Precision = FilterPrecision.Double;
            engine.Configuration.Engine.ReverseTimeAntiClippingAttenuationInDB = 0.0;
            engine.Configuration.Output.BitsPerSample = 16;
            engine.FilterFiles(Environment.CurrentDirectory, TestConstant.SourceFilePath16Bit, outputFileName16Bit);
            this.VerifyWaveFilesEquivalent(outputFileName16Bit, referenceFilePath16Bit, 1.0, 1.0 * TestConstant.Q15ToQ31TruncationErrorIncrease, true);
        }

        private void RemoveExistingOutputFiles(params string[] filesToRemove)
        {
            foreach (string fileToRemove in filesToRemove)
            {
                if (File.Exists(fileToRemove))
                {
                    File.Delete(fileToRemove);
                }
            }
        }

        private void VerifyWaveFilesEquivalent(string actualFilePath, string expectedFilePath, double expectedToActualScaleFactor, double sampleMatchTolerance, bool verifySampleSize)
        {
            // load data in files
            SampleBlock actual;
            WaveFormat actualFormat;
            using (MediaFoundationReader actualReader = new MediaFoundationReader(actualFilePath))
            {
                actual = new SampleBlock((int)actualReader.Length, SampleTypeExtensions.FromBitsPerSample(actualReader.WaveFormat.BitsPerSample));
                actual.BytesInUse = actualReader.Read(actual.ByteBuffer, 0, actual.MaximumSizeInBytes);
                actualFormat = actualReader.WaveFormat;
            }

            SampleBlock expected;
            WaveFormat expectedFormat;
            using (MediaFoundationReader expectedReader = new MediaFoundationReader(expectedFilePath))
            {
                expected = new SampleBlock((int)expectedReader.Length, SampleTypeExtensions.FromBitsPerSample(expectedReader.WaveFormat.BitsPerSample));
                expected.BytesInUse = expectedReader.Read(expected.ByteBuffer, 0, expected.MaximumSizeInBytes);
                expectedFormat = expectedReader.WaveFormat;
            }

            // check data format matches
            Assert.IsTrue(actual.SamplesInUse == expected.SamplesInUse);
            if (verifySampleSize)
            {
                Assert.IsTrue(actual.SampleType == expected.SampleType);
                Assert.IsTrue(actualFormat.AverageBytesPerSecond == expectedFormat.AverageBytesPerSecond);
                Assert.IsTrue(actualFormat.BitsPerSample == expectedFormat.BitsPerSample);
                Assert.IsTrue(actualFormat.BlockAlign == expectedFormat.BlockAlign);
            }
            Assert.IsTrue(actualFormat.Channels == expectedFormat.Channels);
            Assert.IsTrue(actualFormat.Encoding == expectedFormat.Encoding);
            Assert.IsTrue(actualFormat.ExtraSize == expectedFormat.ExtraSize);
            Assert.IsTrue(actualFormat.SampleRate == expectedFormat.SampleRate);

            double largestMismatch = 0.0;
            double maxExpectedValue = Math.Pow(2.0, expected.SampleType.BitsPerSample() - 1) - 1;
            double minExpectedValue = -Math.Pow(2.0, expected.SampleType.BitsPerSample() - 1);
            double mismatchStandardDevation = 0.0;
            using (FileStream samplesStream = new FileStream(String.Format("{0} - {1}.csv", Path.GetFileNameWithoutExtension(actualFilePath), Path.GetFileNameWithoutExtension(expectedFilePath)), FileMode.Create))
            {
                using (StreamWriter samplesWriter = new StreamWriter(samplesStream))
                {
                    samplesWriter.WriteLine("actual,expected");
                    int clippedSamples = 0;
                    int mismatchedSamples = 0;
                    int samples = expected.SamplesInUse;
                    for (int sample = 0; sample < samples; ++sample)
                    {
                        // read samples as ints and convert to doubles regardless of sample type in files
                        double actualSample;
                        switch (actual.SampleType)
                        {
                            case SampleType.Int16:
                                actualSample = actual.ShortBuffer[sample];
                                break;
                            case SampleType.Int24:
                                actualSample = actual.GetInt24AsInt32(sample);
                                break;
                            case SampleType.Int32:
                                actualSample = actual.IntBuffer[sample];
                                break;
                            default:
                                throw new NotSupportedException(String.Format("Unhandled sample type {0}.", expected.SampleType));
                        }
                        actualSample = actualSample / expectedToActualScaleFactor;

                        double expectedSample;
                        switch (expected.SampleType)
                        {
                            case SampleType.Int16:
                                expectedSample = expected.ShortBuffer[sample];
                                break;
                            case SampleType.Int24:
                                expectedSample = expected.GetInt24AsInt32(sample);
                                break;
                            case SampleType.Int32:
                                expectedSample = expected.IntBuffer[sample];
                                break;
                            default:
                                throw new NotSupportedException(String.Format("Unhandled sample type {0}.", expected.SampleType));
                        }
                        samplesWriter.WriteLine("{0},{1}", actualSample, expectedSample);

                        // it's OK if the expected data is clipped and the actual data isn't
                        if (expectedSample >= maxExpectedValue && actualSample > expectedSample)
                        {
                            ++clippedSamples;
                            continue;
                        }
                        if (expectedSample <= minExpectedValue && actualSample < expectedSample)
                        {
                            ++clippedSamples;
                            continue;
                        }

                        // check samples for equivalence
                        // standard deviation calculation is naive as it assumes the average mismatch is zero
                        double mismatch = Math.Abs(expectedSample - actualSample);
                        mismatchStandardDevation += mismatch * mismatch;
                        if (mismatch > largestMismatch)
                        {
                            largestMismatch = mismatch;
                        }
                        if (mismatch > sampleMatchTolerance)
                        {
                            ++mismatchedSamples;
                        }
                    }

                    mismatchStandardDevation = Math.Sqrt(mismatchStandardDevation / (double)expected.SamplesInUse);
                    this.TestContext.WriteLine("Largest mismatch in samples between {0} and {1} was {2:0.0} with a standard deviation of {3:0.0}.", actualFilePath, expectedFilePath, largestMismatch, mismatchStandardDevation);
                    this.TestContext.WriteLine(">>> {0} samples exceeded threshold of {1:0.0}.", mismatchedSamples, sampleMatchTolerance);
                    Assert.IsTrue(mismatchedSamples == 0, "{0} of {1} ({2:#0.0%}) samples did not match within tolerance {3}.", mismatchedSamples, samples, (double)mismatchedSamples / (double)samples, sampleMatchTolerance);
                    if (verifySampleSize)
                    {
                        Assert.IsTrue(clippedSamples < 0.0001 * samples, "{0} (1:#0.000%) samples were clipped; this is unexpectedly high.", clippedSamples, (double)clippedSamples / (double)samples);
                    }
                }
            }

            // check metadata
            Tag actualMetadata;
            using (FileStream inputMetadataStream = new FileStream(actualFilePath, FileMode.Open, FileAccess.Read, FileShare.Read))
            {
                using (TagFile inputTagFile = TagFile.Create(new StreamFileAbstraction(inputMetadataStream.Name, inputMetadataStream, inputMetadataStream)))
                {
                    actualMetadata = inputTagFile.Tag;
                }
            }

            Tag expectedMetadata;
            using (FileStream outputMetadataStream = new FileStream(expectedFilePath, FileMode.Open, FileAccess.Read, FileShare.Read))
            {
                using (TagFile outputTagFile = TagFile.Create(new StreamFileAbstraction(outputMetadataStream.Name, outputMetadataStream, outputMetadataStream)))
                {
                    expectedMetadata = outputTagFile.Tag;
                }
            }

            Assert.AreEqual(expectedMetadata.Album, actualMetadata.Album, String.Format("Expected artist '{0}' but got '{1}'.", expectedMetadata.Album, actualMetadata.Album));
            Assert.AreEqual(expectedMetadata.AmazonId, actualMetadata.AmazonId, String.Format("Expected Amazon ID '{0}' but got '{1}'.", expectedMetadata.AmazonId, actualMetadata.AmazonId));
            Assert.AreEqual(expectedMetadata.Conductor, actualMetadata.Conductor, String.Format("Expected conductor '{0}' but got '{1}'.", expectedMetadata.Conductor, actualMetadata.Conductor));
            Assert.AreEqual(expectedMetadata.Copyright, actualMetadata.Copyright, String.Format("Expected copyright '{0}' but got '{1}'.", expectedMetadata.Copyright, actualMetadata.Copyright));
            Assert.AreEqual(expectedMetadata.Disc, actualMetadata.Disc, String.Format("Expected disc '{0}' but got '{1}'.", expectedMetadata.Disc, actualMetadata.Disc));
            Assert.AreEqual(expectedMetadata.DiscCount, actualMetadata.DiscCount, String.Format("Expected disc count '{0}' but got '{1}'.", expectedMetadata.DiscCount, actualMetadata.DiscCount));
            Assert.AreEqual(expectedMetadata.Grouping, actualMetadata.Grouping, String.Format("Expected grouping '{0}' but got '{1}'.", expectedMetadata.Grouping, actualMetadata.Grouping));
            Assert.AreEqual(expectedMetadata.Lyrics, actualMetadata.Lyrics, String.Format("Expected lyrics '{0}' but got '{1}'.", expectedMetadata.Lyrics, actualMetadata.Lyrics));
            Assert.AreEqual(expectedMetadata.JoinedAlbumArtists, actualMetadata.JoinedAlbumArtists, String.Format("Expected artists '{0}' but got '{1}'.", expectedMetadata.JoinedAlbumArtists, actualMetadata.JoinedAlbumArtists));
            Assert.AreEqual(expectedMetadata.JoinedComposers, actualMetadata.JoinedComposers, String.Format("Expected composers '{0}' but got '{1}'.", expectedMetadata.JoinedComposers, actualMetadata.JoinedComposers));
            Assert.AreEqual(expectedMetadata.JoinedGenres, actualMetadata.JoinedGenres, String.Format("Expected genres '{0}' but got '{1}'.", expectedMetadata.JoinedGenres, actualMetadata.JoinedGenres));
            Assert.AreEqual(expectedMetadata.JoinedPerformersSort, actualMetadata.JoinedPerformersSort, String.Format("Expected performers '{0}' but got '{1}'.", expectedMetadata.JoinedPerformersSort, actualMetadata.JoinedPerformersSort));
            // Pictures is not checked
            Assert.AreEqual(expectedMetadata.Title, actualMetadata.Title, String.Format("Expected title '{0}' but got '{1}'.", expectedMetadata.Title, actualMetadata.Title));
            Assert.AreEqual(expectedMetadata.Track, actualMetadata.Track, String.Format("Expected track '{0}' but got '{1}'.", expectedMetadata.Track, actualMetadata.Track));
            Assert.AreEqual(expectedMetadata.TrackCount, actualMetadata.TrackCount, String.Format("Expected track count '{0}' but got '{1}'.", expectedMetadata.TrackCount, actualMetadata.TrackCount));
            Assert.AreEqual(expectedMetadata.Year, actualMetadata.Year, String.Format("Expected year '{0}' but got '{1}'.", expectedMetadata.Year, actualMetadata.Year));
        }
    }
}
