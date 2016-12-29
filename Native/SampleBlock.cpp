#include "stdafx.h"
#include <assert.h>
#include <iostream>
#include <limits>
#include <new>
#include "Constant.h"
#include "SampleBlock.h"
#include "SampleConverter.h"

namespace CrossTimeDsp::Dsp
{
	SampleBlock::SampleBlock(array<Byte>^ buffer, __int32 bytesInUse, Dsp::SampleType sampleType)
		: SampleBlock(buffer->Length, sampleType)
	{
		pin_ptr<unsigned __int8> sourcePointer = &buffer[0];
		std::memcpy(this->block, sourcePointer, buffer->Length);
		this->bytesInUse = bytesInUse;
	}

	SampleBlock::SampleBlock(__int32 requestedSizeInBytes, Dsp::SampleType sampleType)
	{
		// round size up to filter granularity
		__int32 filterBlocks = requestedSizeInBytes / Constant::FilterBlockSizeInBytes + 1;
		__int32 bytesToAllocate = filterBlocks * Constant::FilterBlockSizeInBytes;

		this->block = (unsigned __int8 *)_aligned_malloc(bytesToAllocate, alignof(__m256d));
		this->bytesAllocated = bytesToAllocate;
		this->bytesInUse = 0;
		this->sampleType = sampleType;
	}

	SampleBlock::!SampleBlock()
	{
		_aligned_free(this->block);
	}

	SampleBlock::~SampleBlock()
	{
		this->!SampleBlock();
	}

	__int32 SampleBlock::BytesPerSample(Dsp::SampleType sampleType)
	{
		switch (sampleType)
		{
		case Dsp::SampleType::Double:
			return 8;
		case Dsp::SampleType::Int16:
			return 2;
		case Dsp::SampleType::Int24:
			return 3;
		case Dsp::SampleType::Int32:
			return 4;
		default:
			throw gcnew NotSupportedException(String::Format("Sample type {0} is not supported.", sampleType));
		}
	}

	SampleBlock^ SampleBlock::ConvertTo(Dsp::SampleType sampleType)
	{
		__int32 bytesInUseAfterConversion = this->BytesPerSample(sampleType) * this->SamplesInUse;
		SampleBlock^ convertedBlock = gcnew SampleBlock(bytesInUseAfterConversion, sampleType);
		this->ConvertTo(convertedBlock);
		return convertedBlock;
	}

	void SampleBlock::ConvertTo(SampleBlock^ block)
	{
		__int32 bytesInUseAfterConversion = this->BytesPerSample(block->sampleType) * this->SamplesInUse;
		assert(block->MaximumSizeInBytes >= bytesInUseAfterConversion);

		switch (this->sampleType)
		{
		case Dsp::SampleType::Double:
			switch (block->sampleType)
			{
			case Dsp::SampleType::Int16:
				SampleConverter::DoubleToQ15(this->Doubles, block->Int16s, this->DoubleSamples);
				break;
			case Dsp::SampleType::Int24:
				SampleConverter::DoubleToQ23(this->Doubles, block->block, this->DoubleSamples);
				break;
			case Dsp::SampleType::Int32:
				SampleConverter::DoubleToQ31(this->Doubles, block->Int32s, this->DoubleSamples);
				break;
			default:
				throw gcnew NotSupportedException(String::Format("Conversion from {0} to {1} is not supported.", this->sampleType, block->sampleType));
			}
			break;
		case Dsp::SampleType::Int16:
			switch (block->sampleType)
			{
			case Dsp::SampleType::Double:
				SampleConverter::Q15ToDouble(this->Int16s, block->Doubles, this->Int16Samples);
				break;
			case Dsp::SampleType::Int32:
				SampleConverter::Q15ToQ31(this->Int16s, block->Int32s, this->Int16Samples);
				break;
			default:
				throw gcnew NotSupportedException(String::Format("Conversion from {0} to {1} is not supported.", this->sampleType, sampleType));
			}
			break;
		case Dsp::SampleType::Int24:
			switch (block->sampleType)
			{
			case Dsp::SampleType::Double:
				SampleConverter::Q23ToDouble(this->block, block->Doubles, this->SamplesInUse);
				break;
			case Dsp::SampleType::Int32:
				SampleConverter::Q23ToQ31(this->block, block->Int32s, this->SamplesInUse);
				break;
			default:
				throw gcnew NotSupportedException(String::Format("Conversion from {0} to {1} is not supported.", this->sampleType, block->sampleType));
			}
			break;
		case Dsp::SampleType::Int32:
			switch (block->sampleType)
			{
			case Dsp::SampleType::Double:
				SampleConverter::Q31ToDouble(this->Int32s, block->Doubles, this->Int32Samples);
				break;
			case Dsp::SampleType::Int16:
				SampleConverter::Q31ToQ15(this->Int32s, block->Int16s, this->Int32Samples);
				break;
			case Dsp::SampleType::Int24:
				SampleConverter::Q31ToQ23(this->Int32s, block->block, this->Int32Samples);
				break;
			default:
				throw gcnew NotSupportedException(String::Format("Conversion from {0} to {1} is not supported.", this->sampleType, block->sampleType));
			}
			break;
		default:
			throw gcnew NotSupportedException(String::Format("Conversion from sample type {0} is not supported.", this->sampleType));
		}

		block->BytesInUse = bytesInUseAfterConversion;
	}

	void SampleBlock::CopyTo(__int32 sourceOffset, array<Byte>^ destination, __int32 destinationOffset, __int32 bytesToCopy)
	{
		pin_ptr<unsigned __int8> destinationPointer = &destination[destinationOffset];
		std::memcpy(destinationPointer, this->block + sourceOffset, bytesToCopy);
	}

	// test hook
	__int32 SampleBlock::GetInt24AsInt32(__int32 sample)
	{
		__int32 blockIndex = 3 * sample;
		__int32 lowByte = this->block[blockIndex] << 8;
		__int32 midByte = this->block[blockIndex + 1] << 16;
		__int32 highByte = this->block[blockIndex + 2] << 24;
		__int32 integer = (highByte | midByte | lowByte) >> 8;
		return integer;
	}

	void SampleBlock::ZeroUnused()
	{
		__int32 bytesToZero = this->bytesAllocated - this->bytesInUse;
		if (bytesToZero > 0)
		{
			std::memset(this->block + this->bytesInUse, 0, bytesToZero);
		}
	}
}