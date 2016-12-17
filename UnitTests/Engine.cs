using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using CrossTimeDsp;
using CrossTimeDsp.Configuration;
using CrossTimeDsp.Dsp;
using CrossTimeDsp.Encodings.Wav;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace Tests
{
    [TestClass]
    public class Engine
    {
        private static readonly double Q15ToQ31TruncationErrorIncrease = Math.Pow(2.0, DspConstants.ShiftBetween16BitSamplesAndQ31);
        private static readonly double Q23ToQ31TruncationErrorIncrease = Math.Pow(2.0, DspConstants.ShiftBetween24BitSamplesAndQ31);
        private static readonly string SourceFilePath16Bit = "whiteNoise.44100.16.wav";

        private BiquadElement CreateBiquad(FilterType type, double f0, double gainInDB, double q)
        {
            BiquadElement biquad = new BiquadElement();
            biquad.F0 = f0;
            biquad.GainInDB = gainInDB;
            biquad.Q = q;
            biquad.TimeDirection = TimeDirection.Forward;
            biquad.Type = type;
            return biquad;
        }

        [TestMethod]
        public void FileCreationAndPreservation()
        {
            CrossTimeEngine engine = new CrossTimeEngine();
            string inputFilePath = Path.Combine(Environment.CurrentDirectory, "fileCreationAndPreservation.wav");
            if (!File.Exists(inputFilePath))
            {
                File.Copy(Engine.SourceFilePath16Bit, inputFilePath);
            }
            string outputFileName = String.Format("{0}{1}.wav", Path.GetFileNameWithoutExtension(inputFilePath), engine.Configuration.Output.CollidingFileNamePostfix);
            this.RemoveExistingOutputFiles(outputFileName);

            // process file and output file should be created
            engine.Configuration.Filters.Filters.Clear();
            engine.FilterFile(inputFilePath);
            DateTime firstOutputFileWriteTime = File.GetLastWriteTimeUtc(outputFileName);
            // filter again and the output file should not be touched
            engine.FilterFile(inputFilePath);
            DateTime secondOutputFileWriteTime = File.GetLastWriteTimeUtc(outputFileName);
            Assert.AreEqual(firstOutputFileWriteTime, secondOutputFileWriteTime, String.Format("Output file {0} was touched after initial write at {1} by second write at {2}.", inputFilePath, firstOutputFileWriteTime, secondOutputFileWriteTime));
        }

        [TestMethod]
        public void ForwardTimeCrossoverAndEQ()
        {
            // set up
            CrossTimeEngine engine = new CrossTimeEngine();
            engine.Configuration.Filters.Filters.Clear();

            string midrangeOutputFileNameDouble = "forwardTimeBandpassAndEQ.midrange.44100.24.double.wav";
            string midrangeOutputFileNameQ31 = "forwardTimeBandpassAndEQ.midrange.44100.24.Q31.wav";
            string midrangeOutputFileNameQ31_32x64 = "forwardTimeBandpassAndEQ.midrange.44100.24.Q31_32x64.wav";
            string midrangeOutputFileNameQ31Adaptive = "forwardTimeBandpassAndEQ.midrange.44100.24.Q31Adaptive.wav";
            string tweeterOutputFileNameDouble = "forwardTimeBandpassAndEQ.tweeter.44100.24.double.wav";
            string tweeterOutputFileNameQ31 = "forwardTimeBandpassAndEQ.tweeter.44100.24.Q31.wav";
            string tweeterOutputFileNameQ31_32x64 = "forwardTimeBandpassAndEQ.tweeter.44100.24.Q31_32x64.wav";
            string subwooferOutputFileNameDouble = "forwardTimeBandpassAndEQ.subwoofer.44100.24.double.wav";
            string subwooferOutputFileNameQ31_32x64 = "forwardTimeBandpassAndEQ.subwoofer.44100.24.Q31_32x64.wav";
            string subwooferOutputFileNameQ31_64x64 = "forwardTimeBandpassAndEQ.subwoofer.44100.24.Q31_64x64.wav";
            string subwooferOutputFileNameQ31Adaptive = "forwardTimeBandpassAndEQ.subwoofer.44100.24.Q31.wav";
            this.RemoveExistingOutputFiles(midrangeOutputFileNameDouble, midrangeOutputFileNameQ31, midrangeOutputFileNameQ31_32x64, midrangeOutputFileNameQ31Adaptive,
                                           tweeterOutputFileNameDouble, tweeterOutputFileNameQ31, tweeterOutputFileNameQ31_32x64,
                                           subwooferOutputFileNameDouble, subwooferOutputFileNameQ31_32x64, subwooferOutputFileNameQ31_64x64, subwooferOutputFileNameQ31Adaptive);

            // midrange
            engine.Configuration.Filters.Filters.Clear();
            // 200Hz LR6 highpass
            engine.Configuration.Filters.Filters.Add(this.CreateBiquad(FilterType.Highpass, 200.0, 0.0, 0.5));
            engine.Configuration.Filters.Filters.Add(this.CreateBiquad(FilterType.Highpass, 200.0, 0.0, 1.0));
            engine.Configuration.Filters.Filters.Add(this.CreateBiquad(FilterType.Highpass, 200.0, 0.0, 1.0));
            // 1700Hz LR6 lowpass
            engine.Configuration.Filters.Filters.Add(this.CreateBiquad(FilterType.Lowpass, 1700.0, 0.0, 0.5));
            engine.Configuration.Filters.Filters.Add(this.CreateBiquad(FilterType.Lowpass, 1700.0, 0.0, 1.0));
            engine.Configuration.Filters.Filters.Add(this.CreateBiquad(FilterType.Lowpass, 1700.0, 0.0, 1.0));
            // dipole EQ
            engine.Configuration.Filters.Filters.Add(this.CreateBiquad(FilterType.Peaking, 150.0, 15.0, 0.5));
            // driver EQ
            engine.Configuration.Filters.Filters.Add(this.CreateBiquad(FilterType.Peaking, 550.0, 4.0, 1.0));

            engine.Configuration.Engine.Precision = FilterPrecision.Double;
            engine.FilterFile(Engine.SourceFilePath16Bit, midrangeOutputFileNameDouble);
            engine.Configuration.Engine.Precision = FilterPrecision.Q31;
            engine.FilterFile(Engine.SourceFilePath16Bit, midrangeOutputFileNameQ31);
            this.VerifyWaveFilesEquivalent(midrangeOutputFileNameQ31, midrangeOutputFileNameDouble, 1.0, 56.0 * Engine.Q23ToQ31TruncationErrorIncrease, true);
            engine.Configuration.Engine.Precision = FilterPrecision.Q31_32x64;
            engine.FilterFile(Engine.SourceFilePath16Bit, midrangeOutputFileNameQ31_32x64);
            this.VerifyWaveFilesEquivalent(midrangeOutputFileNameQ31_32x64, midrangeOutputFileNameDouble, 1.0, 6.0 * Engine.Q23ToQ31TruncationErrorIncrease, true);
            engine.Configuration.Engine.Precision = FilterPrecision.Q31Adaptive;
            engine.FilterFile(Engine.SourceFilePath16Bit, midrangeOutputFileNameQ31Adaptive);
            this.VerifyWaveFilesEquivalent(midrangeOutputFileNameQ31Adaptive, midrangeOutputFileNameDouble, 1.0, 6.0 * Engine.Q23ToQ31TruncationErrorIncrease, true);

            // tweeter
            engine.Configuration.Filters.Filters.Clear();
            // 1700Hz LR6 highpass
            engine.Configuration.Filters.Filters.Add(this.CreateBiquad(FilterType.Highpass, 1700.0, 0.0, 0.5));
            engine.Configuration.Filters.Filters.Add(this.CreateBiquad(FilterType.Highpass, 1700.0, 0.0, 1.0));
            engine.Configuration.Filters.Filters.Add(this.CreateBiquad(FilterType.Highpass, 1700.0, 0.0, 1.0));
            // dipole EQ
            engine.Configuration.Filters.Filters.Add(this.CreateBiquad(FilterType.Peaking, 1400.0, 5.0, 1.9));
            engine.Configuration.Filters.Filters.Add(this.CreateBiquad(FilterType.Peaking, 3800.0, -2.0, 3.0));
            engine.Configuration.Filters.Filters.Add(this.CreateBiquad(FilterType.Peaking, 7300.0, 3.5, 2.0));
            engine.Configuration.Filters.Filters.Add(this.CreateBiquad(FilterType.Peaking, 13000.0, -3.5, 4.0));

            Trace.TraceInformation(String.Empty);
            engine.Configuration.Engine.Precision = FilterPrecision.Double;
            engine.FilterFile(Engine.SourceFilePath16Bit, tweeterOutputFileNameDouble);
            engine.Configuration.Engine.Precision = FilterPrecision.Q31;
            engine.FilterFile(Engine.SourceFilePath16Bit, tweeterOutputFileNameQ31);
            this.VerifyWaveFilesEquivalent(tweeterOutputFileNameQ31, tweeterOutputFileNameDouble, 1.0, 1.0 * Engine.Q23ToQ31TruncationErrorIncrease, true);
            engine.Configuration.Engine.Precision = FilterPrecision.Q31_32x64;
            engine.FilterFile(Engine.SourceFilePath16Bit, tweeterOutputFileNameQ31_32x64);
            this.VerifyWaveFilesEquivalent(tweeterOutputFileNameQ31_32x64, tweeterOutputFileNameDouble, 1.0, 1.0 * Engine.Q23ToQ31TruncationErrorIncrease, true);

            // subwoofer
            engine.Configuration.Filters.Filters.Clear();
            // 200Hz LR6 lowpass
            engine.Configuration.Filters.Filters.Add(this.CreateBiquad(FilterType.Lowpass, 200.0, 0.0, 0.5));
            engine.Configuration.Filters.Filters.Add(this.CreateBiquad(FilterType.Lowpass, 200.0, 0.0, 1.0));
            engine.Configuration.Filters.Filters.Add(this.CreateBiquad(FilterType.Lowpass, 200.0, 0.0, 1.0));
            // dipole EQ
            engine.Configuration.Filters.Filters.Add(this.CreateBiquad(FilterType.Peaking, 39.0, 14.0, 0.5));
            engine.Configuration.Filters.Filters.Add(this.CreateBiquad(FilterType.Peaking, 180.0, 2.0, 1.0));
            // phase linearization
            engine.Configuration.Filters.Filters.Add(this.CreateBiquad(FilterType.Allpass, 150.0, 0.0, 0.5 * Math.Sqrt(2.0)));

            Trace.TraceInformation(String.Empty);
            engine.Configuration.Engine.Precision = FilterPrecision.Double;
            engine.FilterFile(Engine.SourceFilePath16Bit, subwooferOutputFileNameDouble);
            engine.Configuration.Engine.Precision = FilterPrecision.Q31_32x64;
            engine.FilterFile(Engine.SourceFilePath16Bit, subwooferOutputFileNameQ31_32x64);
            this.VerifyWaveFilesEquivalent(subwooferOutputFileNameQ31_32x64, subwooferOutputFileNameDouble, 1.0, 37.0 * Engine.Q23ToQ31TruncationErrorIncrease, true);
            engine.Configuration.Engine.Precision = FilterPrecision.Q31_64x64;
            engine.FilterFile(Engine.SourceFilePath16Bit, subwooferOutputFileNameQ31_64x64);
            this.VerifyWaveFilesEquivalent(subwooferOutputFileNameQ31_64x64, subwooferOutputFileNameDouble, 1.0, 1.0 * Engine.Q23ToQ31TruncationErrorIncrease, true);
            engine.Configuration.Engine.Precision = FilterPrecision.Q31Adaptive;
            engine.FilterFile(Engine.SourceFilePath16Bit, subwooferOutputFileNameQ31Adaptive);
            this.VerifyWaveFilesEquivalent(subwooferOutputFileNameQ31Adaptive, subwooferOutputFileNameDouble, 1.0, 28.0 * Engine.Q23ToQ31TruncationErrorIncrease, true);
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
            double referenceFileScaleFactor = 1.019154;
            CrossTimeEngine engine = new CrossTimeEngine();
            engine.FilterFile(Engine.SourceFilePath16Bit, outputFileName24Bit);
            string referenceFilePath24Bit = "whiteNoise.InverseAllpass.44100.24.wav";
            this.VerifyWaveFilesEquivalent(outputFileName24Bit, referenceFilePath24Bit, referenceFileScaleFactor, 1.5 * Engine.Q23ToQ31TruncationErrorIncrease, true);

            // check cross bitness case - cases in the same bitness are less likely to catch wav parser bugs
            // tolerance here may seem loose but comparing 24 bit samples to their truncated 16 bit form is inexact
            string referenceFilePath16Bit = "whiteNoise.InverseAllpass.44100.16.wav";
            this.VerifyWaveFilesEquivalent(outputFileName24Bit, referenceFilePath16Bit, 0.3162, 40.5 * Engine.Q23ToQ31TruncationErrorIncrease, false);

            // check fixed point
            engine.Configuration.Engine.Precision = FilterPrecision.Q31_32x64;
            engine.FilterFile(Engine.SourceFilePath16Bit, outputFileNameQ31x63);
            this.VerifyWaveFilesEquivalent(outputFileNameQ31x63, referenceFilePath24Bit, referenceFileScaleFactor, 1.6 * Engine.Q23ToQ31TruncationErrorIncrease, true);

            // check 16 bit cases
            engine.Configuration.Engine.Precision = FilterPrecision.Double;
            engine.Configuration.Engine.ReverseTimeAntiClippingAttenuationInDB = 0.0;
            engine.Configuration.Output.Wav.BitsPerSample = 16;
            engine.FilterFile(Engine.SourceFilePath16Bit, outputFileName16Bit);
            this.VerifyWaveFilesEquivalent(outputFileName16Bit, referenceFilePath16Bit, 1.0, 1.0 * Engine.Q15ToQ31TruncationErrorIncrease, true);
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

        private void VerifyWaveFilesEquivalent(string actualFilePath, string expectedFilePath, double scaleFactor, double sampleMatchTolerance, bool checkSampleSize)
        {
            // load files and check header information
            WavStream actual = new WavStream(actualFilePath);
            WavStream expected = new WavStream(expectedFilePath);
            Assert.AreEqual(expected.Channels, actual.Channels, String.Format("Expected {0} channels but got {1}.", expected.Channels, actual.Channels));
            Assert.AreEqual(expected.ChannelMask, actual.ChannelMask, String.Format("Expected channel mask of 0x{0:x} but got 0x{1:x}.", expected.ChannelMask, actual.ChannelMask));
            if (checkSampleSize)
            {
                Assert.AreEqual(expected.SampleSizeInBits, actual.SampleSizeInBits, String.Format("Expected sample size of {0} bits but got {1}.", expected.SampleSizeInBits, actual.SampleSizeInBits));
            }
            Assert.AreEqual(expected.SamplesPerSecond, actual.SamplesPerSecond, String.Format("Expected rate of {0} samples per second but got {1}.", expected.SamplesPerSecond, actual.SamplesPerSecond));
            Assert.AreEqual(expected.TotalSamples, actual.TotalSamples, String.Format("Expected {0} samples but got {1}.", expected.TotalSamples, actual.TotalSamples));

            // check metadata
            Assert.AreEqual(expected.Artist, actual.Artist, String.Format("Expected artist of '{0}' but got '{1}'.", expected.Artist, actual.Artist));
            Assert.AreEqual(expected.Genre, actual.Genre, String.Format("Expected genre of '{0}' but got '{1}'.", expected.Genre, actual.Genre));
            Assert.AreEqual(expected.Name, actual.Name, String.Format("Expected name of '{0}' but got '{1}'.", expected.Name, actual.Name));
            Assert.AreEqual(expected.Product, actual.Product, String.Format("Expected product of '{0}' but got '{1}'.", expected.Product, actual.Product));
            Assert.AreEqual(expected.Track, actual.Track, String.Format("Expected track of '{0}' but got '{1}'.", expected.Track, actual.Track));

            // compare data in files
            // assumes twos complement signed integers
            IEnumerator<Int32> actualEnumerator = actual.GetEnumerator();
            IEnumerator<Int32> expectedEnumerator = expected.GetEnumerator();
            double largestMismatch = 0.0;
            Int32 maxExpectedValue = (Int32)Math.Pow(2.0, expected.SampleSizeInBits - 1) - 1;
            Int32 minExpectedValue = (Int32)Math.Pow(2.0, expected.SampleSizeInBits - 1);
            int mismatchedSamples = 0;
            double mismatchStandardDevation = 0.0;
            FileStream samplesStream = new FileStream(String.Format("{0}-{1}.csv", Path.GetFileNameWithoutExtension(actualFilePath), Path.GetFileNameWithoutExtension(expectedFilePath)), FileMode.Create);
            StreamWriter samplesWriter = new StreamWriter(samplesStream);
            for (int index = 0; index < expected.TotalSamples; ++index)
            {
                // it's OK if the expected data is clipped and the actual data isn't
                actualEnumerator.MoveNext();
                expectedEnumerator.MoveNext();
                double actualSample = (double)actualEnumerator.Current;
                double expectedSample = scaleFactor * (double)expectedEnumerator.Current;
                samplesWriter.WriteLine("{0},{1}", actualSample, expectedSample);
                if (expectedEnumerator.Current >= maxExpectedValue && actualSample > expectedSample)
                {
                    continue;
                }
                if (expectedEnumerator.Current <= minExpectedValue && actualSample < expectedSample)
                {
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
            mismatchStandardDevation = Math.Sqrt(mismatchStandardDevation / (double)expected.TotalSamples);
            Trace.TraceInformation("Largest mismatch in samples between {0} and {1} was {2:0.0} with a standard deviation of {3:0.0}.", actualFilePath, expectedFilePath, largestMismatch, mismatchStandardDevation);
            Trace.TraceInformation(">>> {0} samples exceeded threshold of {1:0.0}.", mismatchedSamples, sampleMatchTolerance);
            if (mismatchedSamples > 0)
            {
                throw new AssertFailedException(String.Format("{0} samples did not match.", mismatchedSamples));
            }
            actual.Close();
            expected.Close();
            samplesWriter.Close();
        }
    }
}
