using System;
using System.Collections.Generic;
using System.IO;
using CrossTimeDsp.Common;
using CrossTimeDsp.Encodings.Ogg;

namespace CrossTimeDsp.Encodings.Flac
{
    internal class FlacConverter
    {
        private const int DefaultBitsPerSample = 24;
        private const int DefaultCompressionLevel = 8;
        private static readonly string MetaflacExecutableName = "metaflac.exe";

        private ProcessExecutor flac;
        private ProcessExecutor metaflac;

        public int CompressionLevel { get; set; }
        public int OutputBitsPerSample { get; set; }

        public FlacConverter(string pathToFlacExe)
        {
            this.CompressionLevel = FlacConverter.DefaultCompressionLevel;
            this.flac = new ProcessExecutor(pathToFlacExe);
            string pathToMetaflacExe = Path.Combine(Path.GetDirectoryName(pathToFlacExe), FlacConverter.MetaflacExecutableName);
            this.metaflac = new ProcessExecutor(pathToMetaflacExe);
            this.OutputBitsPerSample = FlacConverter.DefaultBitsPerSample;
        }

        public Dictionary<MetadataType, string> ConvertFlacToWav(string inputFilePath, string outputFilePath)
        {
            // call metaflac.exe to get metadata
            List<string> metaflacArguments = new List<string>();
            metaflacArguments.Add(String.Format("{0}=-", FlacConstants.MetaflacArguments.ExportTagsTo));
            metaflacArguments.Add(inputFilePath);
            StringReader metadataReader = new StringReader(this.metaflac.Execute(metaflacArguments));
            Dictionary<MetadataType, string> metadata = new Dictionary<MetadataType, string>();
            for (string line = metadataReader.ReadLine(); line != null; line = metadataReader.ReadLine())
            {
                string[] tokens = line.Split('=');
                if (tokens.Length != 2)
                {
                    throw new NotSupportedException(String.Format("Unhandled metadata format in line '{0}'.", line));
                }
                string tag = tokens[0].Trim();
                if (String.Equals(tag, "WAVEFORMATEXTENSIBLE_CHANNEL_MASK", StringComparison.OrdinalIgnoreCase) == false)
                {
                    metadata.Add(this.GetTagType(tag), tokens[1].Trim());
                }
            }
            
            // call flac.exe to decode to .wav
            List<string> flacArguments = new List<string>();
            flacArguments.Add(FlacConstants.FlacArguments.Decode);
            flacArguments.Add(FlacConstants.FlacArguments.Force);
            flacArguments.Add(String.Format("{0}={1}", FlacConstants.FlacArguments.OutputName, outputFilePath));
            flacArguments.Add(inputFilePath);
            this.flac.Execute(flacArguments);

            return metadata;
        }

        public void ConvertWavToFlac(string inputFilePath, string outputFilePath, Dictionary<MetadataType, string> metadata)
        {
            List<string> arguments = new List<string>();
            // specify output bit depth, compression level, and that any existing file should be overwritten
            arguments.Add(String.Format("{0}={1}", FlacConstants.FlacArguments.BitsPerSample, this.OutputBitsPerSample));
            arguments.Add(String.Format("-{0}", this.CompressionLevel));
            arguments.Add(FlacConstants.FlacArguments.Force);
            // specify metadata
            if (metadata != null)
            {
                foreach (KeyValuePair<MetadataType, string> tag in metadata)
                {
                    arguments.Add(String.Format("{0}={1}={2}", FlacConstants.FlacArguments.Tag, this.GetTagName(tag.Key), tag.Value));
                }
                // specify output file name, that flac.exe shouldn't spew progress (no one's watching), and that the output should be verified
                arguments.Add(String.Format("{0}={1}", FlacConstants.FlacArguments.OutputName, outputFilePath));
                arguments.Add(FlacConstants.FlacArguments.Silent);
                arguments.Add(FlacConstants.FlacArguments.Verify);
                // specify the input file to convert
                arguments.Add(inputFilePath);
            }
            this.flac.Execute(arguments);
        }

        private string GetTagName(MetadataType tag)
        {
            switch (tag)
            {
                case MetadataType.Album:
                    return VorbisConstants.Comments.Album;
                case MetadataType.AlbumArtist:
                    return VorbisConstants.Comments.AlbumArtist;
                case MetadataType.Artist:
                    return VorbisConstants.Comments.Artist;
                case MetadataType.Composer:
                    return VorbisConstants.Comments.Composer;
                case MetadataType.Date:
                    return VorbisConstants.Comments.Date;
                case MetadataType.Genre:
                    return VorbisConstants.Comments.Genre;
                case MetadataType.Organization:
                    return VorbisConstants.Comments.Organization;
                case MetadataType.Title:
                    return VorbisConstants.Comments.Title;
                case MetadataType.Track:
                    return VorbisConstants.Comments.TrackNumber;
                case MetadataType.Year:
                    return VorbisConstants.Comments.Year;
                default:
                    throw new NotSupportedException(String.Format("Unhandled tag type {0}.", tag));
            }
        }

        private MetadataType GetTagType(string tag)
        {
            if (String.Equals(tag, VorbisConstants.Comments.Album, StringComparison.OrdinalIgnoreCase))
            {
                return MetadataType.Album;
            }
            if (String.Equals(tag, VorbisConstants.Comments.AlbumArtist, StringComparison.OrdinalIgnoreCase))
            {
                return MetadataType.AlbumArtist;
            }
            if (String.Equals(tag, VorbisConstants.Comments.Artist, StringComparison.OrdinalIgnoreCase))
            {
                return MetadataType.Artist;
            }
            if (String.Equals(tag, VorbisConstants.Comments.Composer, StringComparison.OrdinalIgnoreCase))
            {
                return MetadataType.Composer;
            }
            if (String.Equals(tag, VorbisConstants.Comments.Date, StringComparison.OrdinalIgnoreCase))
            {
                return MetadataType.Date;
            }
            if (String.Equals(tag, VorbisConstants.Comments.Genre, StringComparison.OrdinalIgnoreCase))
            {
                return MetadataType.Genre;
            }
            if (String.Equals(tag, VorbisConstants.Comments.Organization, StringComparison.OrdinalIgnoreCase))
            {
                return MetadataType.Organization;
            }

            if (String.Equals(tag, VorbisConstants.Comments.Title, StringComparison.OrdinalIgnoreCase))
            {
                return MetadataType.Title;
            }
            if (String.Equals(tag, VorbisConstants.Comments.TrackNumber, StringComparison.OrdinalIgnoreCase))
            {
                return MetadataType.Track;
            }
            if (String.Equals(tag, VorbisConstants.Comments.Year, StringComparison.OrdinalIgnoreCase))
            {
                return MetadataType.Year;
            }
            throw new NotSupportedException(String.Format("Unhandled tag '{0}'.", tag));
        }
    }
}
