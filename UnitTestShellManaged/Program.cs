using CrossTimeDsp.UnitTests;

namespace CrossTimeDsp.UnitTestShell
{
    /// <summary>
    /// A minimal shim allowing external profilers and analyzers to access loops in Cross Time DSP without the overhead of launching PowerShell.
    /// </summary>
    public class Program
    {
        public static void Main(string[] args)
        {
            PerformanceTests performanceTests = new PerformanceTests();
            // performanceTests.RunningIterations = 5;
            performanceTests.TestContext = new ShellTestContext();
            // performanceTests.WarmupIterations = 0;

            // performanceTests.FlacToInt24ViaBiquadAndFirstOrderDouble();
            // performanceTests.FlacToInt24ViaThirdOrderDouble();
            performanceTests.FlacToInt24ViaThreeWayLinearizationDouble();
        }
    }
}
