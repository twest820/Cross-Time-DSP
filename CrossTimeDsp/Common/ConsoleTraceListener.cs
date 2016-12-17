using System;
using System.Diagnostics;

namespace CrossTimeDsp.Common
{
    internal class ConsoleTraceListener : System.Diagnostics.ConsoleTraceListener
    {
        public override void TraceEvent(TraceEventCache eventCache, string source, TraceEventType eventType, int id, string message)
        {
            switch (eventType)
            {
                // TraceListener.TraceEvent() includes the source, event type, and id as well as the message
                // this information has no value to users reading the traces on the console, so suppress it
                case TraceEventType.Information:
                    Console.WriteLine(message.PadLeft(message.Length + this.IndentLevel * this.IndentSize));
                    break;
                // if needed, special case other event types
                default:
                    base.TraceEvent(eventCache, source, eventType, id, message);
                    break;
            }
        }

        // nearly identical to the previous overload, but it's improper to use method forwarding as Console.WriteLine(string) and
        // Console.WriteLine(string, params object[]) have different behaviors in how the string is formatted
        public override void TraceEvent(TraceEventCache eventCache, string source, TraceEventType eventType, int id, string format, params object[] args)
        {
            switch (eventType)
            {
                case TraceEventType.Information:
                    Console.WriteLine(format.PadLeft(format.Length + this.IndentLevel * this.IndentSize), args);
                    break;
                default:
                    base.TraceEvent(eventCache, source, eventType, id, format, args);
                    break;
            }
        }
    }
}
