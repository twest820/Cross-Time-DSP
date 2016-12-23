#include "stdafx.h"
#include "Constant.h"
#include "SampleBlock.h"

namespace CrossTimeDsp::Dsp
{
	SampleBlock::SampleBlock(__int32 requestedSizeInBytes, Dsp::SampleType sampleType)
	{
		// round size up to filter granularity
		__int32 filterBlocks = requestedSizeInBytes / Constant::FilterBlockSizeInBytes + 1;
		__int32 bytesToAllocate = filterBlocks * Constant::FilterBlockSizeInBytes;

		this->byteBuffer = gcnew array<Byte>(bytesToAllocate);
		this->bytesInUse = 0;
		this->sampleType = sampleType;
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
		if (this->sampleType == sampleType)
		{
			return this;
		}

		__int32 bytesInUseAfterConversion = this->BytesPerSample(sampleType) * this->SamplesInUse;
		SampleBlock^ convertedBlock = gcnew SampleBlock(bytesInUseAfterConversion, sampleType);
		switch (this->sampleType)
		{
		case Dsp::SampleType::Double:
			switch (sampleType)
			{
			case Dsp::SampleType::Int16:
				this->ConvertDoubleToQ15(convertedBlock);
				break;
			case Dsp::SampleType::Int24:
				this->ConvertDoubleToQ23(convertedBlock);
				break;
			case Dsp::SampleType::Int32:
				this->ConvertDoubleToQ31(convertedBlock);
				break;
			default:
				throw gcnew NotSupportedException(String::Format("Conversion from {0} to {1} is not supported.", this->sampleType, sampleType));
			}
			break;
		case Dsp::SampleType::Int16:
			switch (sampleType)
			{
			case Dsp::SampleType::Double:
				this->ConvertQ15ToDouble(convertedBlock);
				break;
			case Dsp::SampleType::Int32:
				this->ConvertQ15ToQ31(convertedBlock);
				break;
			default:
				throw gcnew NotSupportedException(String::Format("Conversion from {0} to {1} is not supported.", this->sampleType, sampleType));
			}
			break;
		case Dsp::SampleType::Int24:
			switch (sampleType)
			{
			case Dsp::SampleType::Double:
				this->ConvertQ23ToDouble(convertedBlock);
				break;
			case Dsp::SampleType::Int32:
				this->ConvertQ23ToQ31(convertedBlock);
				break;
			default:
				throw gcnew NotSupportedException(String::Format("Conversion from {0} to {1} is not supported.", this->sampleType, sampleType));
			}
			break;
		case Dsp::SampleType::Int32:
			switch (sampleType)
			{
			case Dsp::SampleType::Double:
				this->ConvertQ31ToDouble(convertedBlock);
				break;
			case Dsp::SampleType::Int16:
				this->ConvertQ31ToQ15(convertedBlock);
				break;
			case Dsp::SampleType::Int24:
				this->ConvertQ31ToQ23(convertedBlock);
				break;
			default:
				throw gcnew NotSupportedException(String::Format("Conversion from {0} to {1} is not supported.", this->sampleType, sampleType));
			}
			break;
		default:
			throw gcnew NotSupportedException(String::Format("Conversion from sample type {0} is not supported.", this->sampleType));
		}

		convertedBlock->BytesInUse = bytesInUseAfterConversion;
		return convertedBlock;
	}

	__int32 SampleBlock::GetInt24AsInt32(__int32 sample)
	{
		__int32 blockIndex = 3 * sample;
		array<Byte>^ q24 = gcnew array<Byte> { this->ByteBuffer[blockIndex], this->ByteBuffer[blockIndex + 1], this->ByteBuffer[blockIndex + 2], 0 };
		if (q24[2] >= 0x80)
		{
			q24[3] = 0xff;
		}

		return BitConverter::ToInt32(q24, 0);
	}

	void SampleBlock::ConvertDoubleToQ15(SampleBlock^ convertedBlock)
	{
		for (__int32 sample = 0; sample < this->DoubleBufferCount; ++sample)
		{
			__int32 value = (__int32)this->DoubleBuffer[sample] >> Constant::ShiftBetween16BitSamplesAndQ31;
			if (value > Int16::MaxValue)
			{
				convertedBlock->ShortBuffer[sample] = Int16::MaxValue;
			}
			else if (value < Int16::MinValue)
			{
				convertedBlock->ShortBuffer[sample] = Int16::MinValue;
			}
			else
			{
				convertedBlock->ShortBuffer[sample] = (__int16)value;
			}
		}
	}

	void SampleBlock::ConvertDoubleToQ23(SampleBlock^ convertedBlock)
	{
		for (__int32 sample = 0; sample < this->DoubleBufferCount; ++sample)
		{
			this->SetInt32AsInt24((__int32)this->DoubleBuffer[sample] >> Constant::ShiftBetween24BitSamplesAndQ31, convertedBlock, sample);
		}
	}

	void SampleBlock::ConvertDoubleToQ31(SampleBlock^ convertedBlock)
	{
		for (__int32 sample = 0; sample < this->DoubleBufferCount; ++sample)
		{
			double value = this->DoubleBuffer[sample];
			if (value > Int32::MaxValue)
			{
				convertedBlock->IntBuffer[sample] = Int32::MaxValue;
			}
			else if (value < Int32::MinValue)
			{
				convertedBlock->IntBuffer[sample] = Int32::MinValue;
			}
			else
			{
				convertedBlock->IntBuffer[sample] = (__int32)value;
			}
		}
	}

	void SampleBlock::ConvertQ15ToDouble(SampleBlock^ convertedBlock)
	{
		for (__int32 sample = 0; sample < this->ShortBufferCount; ++sample)
		{
			convertedBlock->DoubleBuffer[sample] = (__int64)this->ShortBuffer[sample] << Constant::ShiftBetween16BitSamplesAndQ31;
		}
	}

	void SampleBlock::ConvertQ15ToQ31(SampleBlock^ convertedBlock)
	{
		for (__int32 sample = 0; sample < this->ShortBufferCount; ++sample)
		{
			convertedBlock->IntBuffer[sample] = (__int32)this->ShortBuffer[sample] << Constant::ShiftBetween16BitSamplesAndQ31;
		}
	}

	void SampleBlock::ConvertQ23ToDouble(SampleBlock^ convertedBlock)
	{
		for (__int32 sample = 0; sample < this->DoubleBufferCount; ++sample)
		{
			convertedBlock->DoubleBuffer[sample] = (double)((__int64)this->GetInt24AsInt32(sample) << Constant::ShiftBetween24BitSamplesAndQ31);
		}
	}

	void SampleBlock::ConvertQ23ToQ31(SampleBlock^ convertedBlock)
	{
		for (__int32 sample = 0; sample < this->IntBufferCount; ++sample)
		{
			convertedBlock->IntBuffer[sample] = this->GetInt24AsInt32(sample) << Constant::ShiftBetween24BitSamplesAndQ31;
		}
	}

	void SampleBlock::ConvertQ31ToDouble(SampleBlock^ convertedBlock)
	{
		for (__int32 sample = 0; sample < this->IntBufferCount; ++sample)
		{
			convertedBlock->DoubleBuffer[sample] = this->IntBuffer[sample];
		}
	}

	void SampleBlock::ConvertQ31ToQ15(SampleBlock^ convertedBlock)
	{
		for (__int32 sample = 0; sample < this->IntBufferCount; ++sample)
		{
			__int32 value = this->IntBuffer[sample] >> Constant::ShiftBetween16BitSamplesAndQ31;
			if (value > Int16::MaxValue)
			{
				convertedBlock->ShortBuffer[sample] = Int16::MaxValue;
			}
			else if (value < Int16::MinValue)
			{
				convertedBlock->ShortBuffer[sample] = Int16::MinValue;
			}
			else
			{
				convertedBlock->ShortBuffer[sample] = (__int16)value;
			}
		}
	}

	void SampleBlock::ConvertQ31ToQ23(SampleBlock^ convertedBlock)
	{
		for (__int32 sample = 0; sample < this->IntBufferCount; ++sample)
		{
			this->SetInt32AsInt24(this->IntBuffer[sample] >> Constant::ShiftBetween24BitSamplesAndQ31, convertedBlock, sample);
		}
	}

	void SampleBlock::SetInt32AsInt24(__int32 value, SampleBlock^ convertedBlock, __int32 sample)
	{
		__int32 blockIndex = 3 * sample;
		if (value > Constant::Int24::Int24::MaxValue)
		{
			convertedBlock->ByteBuffer[blockIndex] = Constant::Int24::Int24::MaxValueBytes[0];
			convertedBlock->ByteBuffer[blockIndex + 1] = Constant::Int24::Int24::MaxValueBytes[1];
			convertedBlock->ByteBuffer[blockIndex + 2] = Constant::Int24::Int24::MaxValueBytes[2];
		}
		else if (value < Constant::Int24::Int24::MinValue)
		{
			convertedBlock->ByteBuffer[blockIndex] = Constant::Int24::Int24::MinValueBytes[0];
			convertedBlock->ByteBuffer[blockIndex + 1] = Constant::Int24::Int24::MinValueBytes[1];
			convertedBlock->ByteBuffer[blockIndex + 2] = Constant::Int24::Int24::MinValueBytes[2];
		}
		else
		{
			array<Byte>^ q32Bytes = BitConverter::GetBytes(value);
			convertedBlock->ByteBuffer[blockIndex] = q32Bytes[0];
			convertedBlock->ByteBuffer[blockIndex + 1] = q32Bytes[1];
			convertedBlock->ByteBuffer[blockIndex + 2] = q32Bytes[2];
		}
	}
}