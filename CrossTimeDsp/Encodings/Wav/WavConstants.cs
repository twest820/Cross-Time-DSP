namespace CrossTimeDsp.Encodings.Wav
{
    internal static class WavConstants
    {
        public const int ChunkAlignmentInBytes = 2;
        public const int ChunkIDSize = 4;
        public const int ChunkIDAndSizeSize = WavConstants.ChunkIDSize + 4;

        public static class Chunks
        {
            public const string Artist = "IART";
            public const string Data = "data";
            public const string Fact = "fact";
            public const string Format = "fmt ";
            public const string Genre = "IGNR";
            public const string Itoc = "ITOC";
            public const string List = "LIST";
            public const string Name = "INAM";
            public const string Product = "IPRD";
            // some implementations may use itrk
            // if needed, add support for the lower case version
            public const string Track = "ITRK";
        }

        public const string FileExtension = ".wav";
        public const int GuidSize = 16;
        public const string Info = "INFO";
        public const string KSDataFormatSubTypePcm = "00000001-0000-0010-8000-00aa00389b71";
        public const string Riff = "RIFF";
        public const string Wave = "WAVE";
        public const int WavFormatSize = 16;
        public const int WavFormatExSize = 18;
        public const int WavFormatExtensibleSize = 40;
    }
}
