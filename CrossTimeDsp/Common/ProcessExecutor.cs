using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Text;

namespace CrossTimeDsp.Common
{
    internal class ProcessExecutor
    {
        private static readonly TimeSpan DefaultExecutionTimeout = TimeSpan.FromMinutes(5);
        private string executablePath;
        public TimeSpan ExecutionTimeout { get; set; }

        public ProcessExecutor(string executablePath)
        {
            this.executablePath = executablePath;
            this.ExecutionTimeout = ProcessExecutor.DefaultExecutionTimeout;
        }

        private string EscapeArgument(string argument)
        {
            return argument.Replace("\"", "\\\"");
        }

        public string Execute(List<string> arguments)
        {
            Process process = new Process();
            process.StartInfo = new ProcessStartInfo(this.executablePath);
            if (arguments != null && arguments.Count > 0)
            {
                StringBuilder argumentBuilder = new StringBuilder(String.Format("\"{0}\"", this.EscapeArgument(arguments[0])));
                for (int argument = 1; argument < arguments.Count; ++argument)
                {
                    argumentBuilder.AppendFormat(" \"{0}\"", this.EscapeArgument(arguments[argument]));
                }
                process.StartInfo.Arguments = argumentBuilder.ToString();
            }
            process.StartInfo.CreateNoWindow = true;
            process.StartInfo.ErrorDialog = false;
            process.StartInfo.RedirectStandardOutput = true;
            process.StartInfo.UseShellExecute = false;
            if (process.Start() == false)
            {
                throw new ApplicationException(String.Format("Failed to start '\"{0}\" {1}'", this.executablePath, process.StartInfo.Arguments));
            }
            // if needed, capture standard error as well (requires async read to avoid deadlocks)
            string standardOutput = process.StandardOutput.ReadToEnd();
            if (process.WaitForExit((int)this.ExecutionTimeout.TotalMilliseconds) == false)
            {
                throw new ApplicationException(String.Format("Task '\"{0}\" {1}' failed to complete in {2}.", this.executablePath, process.StartInfo.Arguments, this.ExecutionTimeout));
            }
            if (process.ExitCode != 0)
            {
                throw new ApplicationException(String.Format("Task '\"{0}\" {1}' exited with code {2}.\nStandard output was '{3}'.", this.executablePath, process.StartInfo.Arguments, process.ExitCode, standardOutput));
            }
            return standardOutput;
        }
    }
}
