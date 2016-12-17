using System;

namespace CrossTimeDsp.Encodings.Wav
{
    // not suitable for use with Read/WriteStruct() as ID must be serialized as four ASCII characters
    internal struct Chunk
    {
        public string ID;
        public UInt32 Size;
        // the chunk data could be included in this structure but doing so is unattractive for large chunks which are best streamed
        // instead, it's left to the caller to read from the stream containing the chunk data as needed
        // public byte[] Data;

        // convenience constructor as most .NET APIs use ints for lengths rather than uints
        public Chunk(string id, int size)
            : this(id, (UInt32)size)
        {
        }

        public Chunk(string id, UInt32 size)
        {
            this.ID = id;
            this.Size = size;
        }
    }
}
