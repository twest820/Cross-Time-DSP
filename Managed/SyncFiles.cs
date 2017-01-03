using System;
using System.IO;
using System.Management.Automation;
using System.Threading;
using System.Threading.Tasks;

namespace CrossTimeDsp
{
    [Cmdlet(VerbsData.Sync, "Files")]
    public class SyncFiles : PSCmdlet
    {
        private CrossTimeEngine dspEngine;
        private CrossTimeEngineLog dspEngineLog;

        [Parameter(Mandatory = true, HelpMessage = "Path to configuration file indicating processing to perform.  See ExampleConfig.xml for details.")]
        public string Config { get; set; }

        [Parameter(Mandatory = true, HelpMessage = "Path to input directory, file, or a wildcard specification.  For example, \"Music\\*.flac\" will process all FLAC files under the Music directory.")]
        public string Input { get; set; }

        [Parameter(HelpMessage = "Path to output directory.  If not specified, the input directory is used.")]
        public string Output { get; set; }

        public SyncFiles()
        {
            this.dspEngine = null;
            this.dspEngineLog = new CrossTimeEngineLog();
        }

        protected override void ProcessRecord()
        {
            // get config file, input directory/files, and output directory
            string searchPattern = Path.GetFileName(this.Input);
            this.Input = Path.GetDirectoryName(this.Input);
            if (String.IsNullOrEmpty(this.Input))
            {
                this.Input = this.SessionState.Path.CurrentFileSystemLocation.Path;
            }

            if (String.IsNullOrEmpty(this.Output))
            {
                this.Output = this.Input;
            }

            // canononicalize any paths relative to the current PowerShell file system location as Environment.CurrentDirectory is usually elsewhere
            this.Config = this.RootPath(this.Config);
            this.Input = this.RootPath(this.Input);
            this.Output = this.RootPath(this.Output);

            using (this.dspEngine = new CrossTimeEngine(this.Config, this.dspEngineLog))
            {
                // process specified files
                DateTime startTimeUtc = DateTime.UtcNow;
                Task filterFiles = Task.Run(() => { dspEngine.FilterFiles(this.Input, searchPattern, this.Output); });

                // rely on polling for log messages as Progress<T>.Report() crashes PowerShell
                while (filterFiles.IsCompleted == false)
                {
                    Thread.Sleep(Constant.PowerShellMessagePollInterval);
                    this.WriteMessages();
                }

                // flush any remaining log messages
                this.WriteMessages();
                TimeSpan elapsedTime = DateTime.UtcNow - startTimeUtc;
                this.WriteVerbose(String.Format("Completed in {0}.", elapsedTime.ToString(Constant.ElapsedTimeFormat)));
            }
        }

        private string RootPath(string path)
        {
            if (Path.IsPathRooted(path))
            {
                return path;
            }

            path = Path.Combine(this.SessionState.Path.CurrentFileSystemLocation.Path, path);
            return Path.GetFullPath(path);
        }

        protected override void StopProcessing()
        {
            base.StopProcessing();

            if (this.dspEngine != null)
            {
                this.dspEngine.Stopping = true;
            }
        }

        private void WriteMessages()
        {
            string message;
            while (this.dspEngineLog.Errors.TryTake(out message))
            {
                this.WriteError(new ErrorRecord(new ApplicationException(message), "CrossTimeEngine", ErrorCategory.NotSpecified, this.dspEngine));
            }

            while (this.dspEngineLog.Verbose.TryTake(out message))
            {
                this.WriteVerbose(message);
            }
        }
    }
}
