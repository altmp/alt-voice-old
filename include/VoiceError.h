#pragma once

enum AltVoiceError
{
	Ok,
	DeviceOpenError,
	ContextSetError,
	SourcesCreateError,
	BufferCreateError_InvalidName,
	BufferCreateError_InvalidEnum,
	BufferCreateError_InvalidValue,
	BufferCreateError_InvalidOperation,
	BufferCreateError_OutOfMemory,
	OpusEncoderCreateError,
	OpusDecoderCreateError,
	OpusBitrateSetError,
	DenoiseInitError
};
