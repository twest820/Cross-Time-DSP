using System;
using System.Diagnostics;
using System.IO;
using CrossTimeDsp.Encodings.Wav;

namespace CrossTimeDsp
{
    internal class Program
    {
        public static void Main(string[] args)
        {
            try
            {
                Program program = new Program();
                program.Run(args);
            }
            catch (Exception exception)
            {
                Trace.TraceError(exception.ToString());
            }
        }

        private void Run(string[] args)
        {
            // if this looks like a valid command line, try to process it
            if (args.Length == 1 || args.Length == 2)
            {
                // get input directory and set of files
                // by default, assume args[0] points to a directory
                string inputDirectoryPath = args[0];
                string searchPattern = String.Format("*{0}", WavConstants.FileExtension);
                // if args[0] specifies a set of files rather than a directory, change the search pattern to match the files
                if (inputDirectoryPath.IndexOfAny(new char[] { '*', '?' }) > -1)
                {
                    searchPattern = Path.GetFileName(args[0]);
                    inputDirectoryPath = Path.GetDirectoryName(args[0]);
                    if (String.IsNullOrEmpty(inputDirectoryPath))
                    {
                        inputDirectoryPath = Environment.CurrentDirectory;
                    }
                }
                // if args[0] specifies a file then do single file processing and return
                // this check is done after the check for wildcards as GetAttributes() considers wildcards invalid characters
                CrossTimeEngine dspEngine = new CrossTimeEngine();
                if ((File.GetAttributes(inputDirectoryPath) & FileAttributes.Directory) != FileAttributes.Directory)
                {
                    if (args.Length == 2)
                    {
                        dspEngine.FilterFile(args[0], args[1]);
                    }
                    else
                    {
                        dspEngine.FilterFile(args[0]);
                    }
                    return;
                }
                // get output path
                string outputPath = inputDirectoryPath;
                if (args.Length == 2)
                {
                    outputPath = args[1];
                }
                // process specified files
                dspEngine.FilterFiles(inputDirectoryPath, searchPattern, outputPath);
            }
            else
            {
                // otherwise, print command line syntax
                Trace.TraceInformation("Loads the input file, applies the filters specified in CrossTimeDsp.exe.config, and writes the result to the specified output.");
                Trace.TraceInformation(String.Empty);
                Trace.TraceInformation("CrossTimeDsp.exe <input path> [<output path>]");
                Trace.Indent();
                Trace.TraceInformation("Processes all specified files located in the directory structure below <input");
                Trace.TraceInformation("path> and writes the results to a matching directory structure below <output");
                Trace.TraceInformation("path>.  If <output path> is not specified it's defaulted to the directory");
                Trace.TraceInformation("specified by <input path>.");
                Trace.TraceInformation(String.Empty);
                Trace.TraceInformation("<input path> and <output path> can be file names as well as directories.");
                Trace.TraceInformation("<input path> may contain file name wildcards.  If no wildcard is specified then");
                Trace.TraceInformation(" *.wav will be proccessed.  Wildcard directories are not supported.");
                Trace.Unindent();
            }
        }
    }
}
