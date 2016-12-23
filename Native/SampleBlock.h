#pragma once
#include "SampleType.h"

using namespace System;
using namespace System::Runtime::InteropServices;

namespace CrossTimeDsp::Dsp
{
	// #pragma warning (disable : CA1900)
	[StructLayout(LayoutKind::Explicit, Pack = 4)]
	public ref class SampleBlock
	{
	private:
		[FieldOffset(0)]
		__int32 bytesInUse;

		[FieldOffset(4)]
		SampleType sampleType;

		[FieldOffset(8)]
		array<Byte>^ byteBuffer;
		[FieldOffset(8)]
		array<double>^ doubleBuffer;
		[FieldOffset(8)]
		array<__int32>^ intBuffer;
		[FieldOffset(8)]
		array<__int16>^ shortBuffer;

	public:
		SampleBlock(__int32 requestedSizeInBytes, SampleType sampleType);

		property array<Byte>^ ByteBuffer
		{
			array<Byte>^ get() { return this->byteBuffer; }
		}

		property __int32 ByteBufferCount
		{
			__int32 get() { return this->bytesInUse; }
		}

		property __int32 BytesInUse
		{
			__int32 get() { return this->bytesInUse; }
			void set(__int32 value) { this->bytesInUse = value; }
		}

		property array<double>^ DoubleBuffer
		{
			array<double>^ get() { return this->doubleBuffer; }
		}

		property __int32 DoubleBufferCount
		{
			__int32 get() { return this->bytesInUse / 8; }
		}

		property array<__int32>^ IntBuffer
		{
			array<__int32>^ get() { return this->intBuffer; }
		}

		property __int32 IntBufferCount
		{
			__int32 get() { return this->bytesInUse / 4; }
		}

		property __int32 MaximumSizeInBytes
		{
			__int32 get() { return this->byteBuffer->Length; }
		}

		property __int32 SamplesInUse
		{
			__int32 get() { return this->BytesInUse / this->BytesPerSample(this->sampleType); }
		}

		property SampleType SampleType
		{
			Dsp::SampleType get() { return this->sampleType; }
			void set(Dsp::SampleType value) { this->sampleType = value; }
		}

		property array<__int16>^ ShortBuffer
		{
			array<__int16>^ get() { return this->shortBuffer; }
		}

		property __int32 ShortBufferCount
		{
			__int32 get() { return this->bytesInUse / 2; }
		}

		SampleBlock^ ConvertTo(Dsp::SampleType sampleType);
		__int32 GetInt24AsInt32(__int32 sample);

	private:
		__int32 BytesPerSample(Dsp::SampleType sampleType);

		void ConvertDoubleToQ15(SampleBlock^ convertedBlock);
		void ConvertDoubleToQ23(SampleBlock^ convertedBlock);
		void ConvertDoubleToQ31(SampleBlock^ convertedBlock);
		void ConvertQ15ToDouble(SampleBlock^ convertedBlock);
		void ConvertQ15ToQ31(SampleBlock^ convertedBlock);
		void ConvertQ23ToDouble(SampleBlock^ convertedBlock);
		void ConvertQ23ToQ31(SampleBlock^ convertedBlock);
		void ConvertQ31ToDouble(SampleBlock^ convertedBlock);
		void ConvertQ31ToQ15(SampleBlock^ convertedBlock);
		void ConvertQ31ToQ23(SampleBlock^ convertedBlock);

		void SetInt32AsInt24(__int32 value, SampleBlock^ convertedBlock, __int32 sample);
	};
}