using System;

namespace CrossTimeDsp.Dsp
{
    internal class StreamPerformance
    {
        public DateTime CompleteTimeUtc { get; set; }
        public bool HasForwardTime { get; set; }
        public DateTime ReverseBufferCompleteUtc { get; set; }
        public DateTime ReverseTimeCompleteUtc { get; set; }
        public DateTime StartTimeUtc { get; private set; }

        public StreamPerformance()
        {
            this.CompleteTimeUtc = DateTime.UtcNow;
            this.HasForwardTime = false;
            this.ReverseBufferCompleteUtc = DateTime.MinValue;
            this.ReverseTimeCompleteUtc = DateTime.MinValue;
            this.StartTimeUtc = this.CompleteTimeUtc;
        }

        public bool HasReverseTime
        {
            get { return (this.ReverseBufferCompleteUtc != DateTime.MinValue) && (this.ReverseTimeCompleteUtc != DateTime.MaxValue); }
        }
    }
}
