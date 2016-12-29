#pragma once
#include "SampleType.h"

using namespace System;

namespace CrossTimeDsp::Dsp
{
	public ref class SampleBlock
	{
	private:
		unsigned __int8* block;
		__int32 bytesAllocated;
		__int32 bytesInUse;
		SampleType sampleType;

	public:
		SampleBlock(array<Byte>^ buffer, __int32 bytesInUse, SampleType sampleType);
		SampleBlock(__int32 requestedSizeInBytes, SampleType sampleType);
		!SampleBlock();
		~SampleBlock();

		property __int32 BytesInUse
		{
			__int32 get() { return this->bytesInUse; }
			void set(__int32 value) { this->bytesInUse = value; }
		}

		property double* Doubles
		{
			double* get() { return reinterpret_cast<double *>(this->block); }
		}

		property __int32 DoubleSamples
		{
			__int32 get() { return this->bytesInUse / 8; }
		}

		property __int16* Int16s
		{
			__int16* get() { return reinterpret_cast<__int16*>(this->block); }
		}

		property __int32 Int16Samples
		{
			__int32 get() { return this->bytesInUse / 2; }
		}

		property __int32* Int32s
		{
			__int32* get() { return reinterpret_cast<__int32*>(this->block); }
		}

		property __int32 Int32Samples
		{
			__int32 get() { return this->bytesInUse / 4; }
		}

		property __int32 MaximumSizeInBytes
		{
			__int32 get() { return this->bytesAllocated; }
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

		__int32 BytesPerSample(Dsp::SampleType sampleType);
		SampleBlock^ ConvertTo(Dsp::SampleType sampleType);
		void SampleBlock::ConvertTo(SampleBlock^ block);
		void SampleBlock::CopyTo(__int32 sourceOffset, array<Byte>^ destination, __int32 destinationOffset, __int32 bytesToCopy);
		__int32 GetInt24AsInt32(__int32 sample);
		void ZeroUnused();
	};
}