namespace CrossTimeDsp.Encodings.Flac
{
    internal static class FlacConstants
    {
        public const string FileExtension = ".flac";

        // command line arguments to flac.exe
        public static class FlacArguments
        {
            public const string BitsPerSample = "--bps";
            public const string Decode = "-d";
            public const string Force = "--force";
            public const string OutputName = "--output-name";
            public const string Silent = "--silent";
            public const string Tag = "--tag";
            public const string Verify = "--verify";
        }

        public static class MetaflacArguments
        {
            public const string ExportTagsTo = "--export-tags-to";
        }
    }
}
