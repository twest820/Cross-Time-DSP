using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace CrossTimeDsp.UnitTests
{
    public class CrossTimeTest : CrossTimeEngineLog
    {
        public TestContext TestContext { get; set; }

        public override void ReportError(string format, params object[] args)
        {
            this.TestContext.WriteLine(format, args);
        }

        public override void ReportVerbose(string format, params object[] args)
        {
            this.TestContext.WriteLine(format, args);
        }
    }
}
