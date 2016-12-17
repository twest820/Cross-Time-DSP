using System.Collections.Generic;
using CrossTimeDsp.Common;

namespace CrossTimeDsp.Encodings.MP3
{
    internal class MP3Converter
    {
        private ProcessExecutor lame;

        public MP3Converter(string pathToLameExe)
        {
            this.lame = new ProcessExecutor(pathToLameExe);
        }

        public void ConvertMP3ToWav(string inputFilePath, string outputFilePath)
        {
            // call lame.exe to decode to .wav
            List<string> lameArguments = new List<string>();
            lameArguments.Add(MP3Constants.LameArguments.Decode);
            lameArguments.Add(inputFilePath);
            lameArguments.Add(outputFilePath);
            this.lame.Execute(lameArguments);
        }
    }
}
