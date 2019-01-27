#pragma once

enum AltVoiceError
{
	Ok,
	DeviceOpenError,
	ContextSetError,
	SourcesCreateError,
	BufferCreateError,
	OpusEncoderCreateError,
	OpusDecoderCreateError,
	OpusBitrateSetError
};
