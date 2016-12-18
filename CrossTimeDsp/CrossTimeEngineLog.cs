using System;
using System.Collections.Concurrent;

namespace CrossTimeDsp
{
    public class CrossTimeEngineLog
    {
        public ConcurrentBag<string> Errors { get; private set; }
        public ConcurrentBag<string> Verbose { get; private set; }

        public CrossTimeEngineLog()
        {
            this.Errors = new ConcurrentBag<string>();
            this.Verbose = new ConcurrentBag<string>();
        }

        public virtual void ReportError(string format, params object[] args)
        {
            this.Errors.Add(String.Format(format, args));
        }

        public virtual void ReportVerbose(string format, params object[] args)
        {
            this.Verbose.Add(String.Format(format, args));
        }
    }
}
