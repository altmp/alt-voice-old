#include <alt-voice.h>

#include <Windows.h>
#include <iostream>
#include <thread>
#include <string>
#include <regex>

std::vector<std::string> split(const std::string str, const std::string token = " ")
{
	return std::vector<std::string>(std::sregex_token_iterator(str.begin(), str.end(), (const std::regex&)std::regex(token), -1), std::sregex_token_iterator());
}

std::thread* inputThread;

int sampleRate = 48000;
int framesPerBuffer = 960;
int bitrate = 64000;
uint16_t opusBuffer[960]{ 0 };
uint16_t pcmBuffer[960]{ 0 };

ISoundInput* soundInput;
ISoundOutput* soundOutput;
IOpusEncoder* opusEncoder = nullptr;
IOpusDecoder* opusDecoder = nullptr;

std::atomic<bool> opusTest = false;

void OnVoiceInput(const void* buffer, int length, float micLevel)
{
	if (opusTest)
	{
		int outLen = opusEncoder->EncodeShort((void*)buffer, length / 2, opusBuffer, 960);
		int outBufferSize = opusDecoder->DecodeShort((void*)opusBuffer, outLen, pcmBuffer, 960);
		soundOutput->Write((void*)pcmBuffer, outBufferSize);
	}
	else
	{
		soundOutput->Write((void*)buffer, length / 2);
	}
}

BOOL WINAPI ConsoleHandler(DWORD dwCtrlType)
{
	if (dwCtrlType == CTRL_CLOSE_EVENT)
	{
		return TRUE;
	}
	return FALSE;
}

int main()
{
	printf("-----------------------alt:Voice device tester-----------------------\n");
	printf("Available commands:\n");
	printf("outputlist\t\t- list of the output devices (id-name)\n");
	printf("inputlist\t\t- list of the input devices (id-name)\n");
	printf("setoutput <id>\t\t- set output device by id\n");
	printf("setinput <id>\t\t- set input device by id\n");
	printf("outputvol <0-200>\t- set output volume\n");
	printf("inputvol <0-200>\t- set input volume\n");
	printf("noise <0-1>\t\t- toggle noise cancellation\n");
	printf("normalize <0-1>\t\t- toggle mic input normalization\n");
	printf("opus <0-1>\t\t- toggle opus encoding-decoding\n");
	printf("toggleinput <0-1>\t\t- toggle mic input\n");
	printf("---------------------------------------------------------------------\n");

	SetConsoleCtrlHandler(ConsoleHandler, TRUE);
	SetConsoleCP(CP_UTF8);
	SetConsoleOutputCP(CP_UTF8);

	CreateOpusEncoder(48000, 1, &opusEncoder, bitrate);
	CreateOpusDecoder(48000, 1, &opusDecoder);

	CreateSoundOutput(sampleRate, framesPerBuffer, bitrate, &soundOutput);
	soundOutput->ChangeOutputVolume(1.f);

	CreateSoundInput(sampleRate, framesPerBuffer, bitrate, &soundInput);
	soundInput->SetNoiseSuppressionStatus(false);
	soundInput->SetNormalizationEnabled(false);
	soundInput->ChangeMicGain(1.f);
	soundInput->RegisterRawCallback(OnVoiceInput);
	soundInput->EnableInput();

	inputThread = new std::thread([]() {
		std::string input;
		while (std::getline(std::cin, input))
		{
			if (input.find("outputvol") != std::string::npos)
			{
				auto out = split(input);
				auto volume = std::stof(out[1]);
				if (volume < 0.f || volume > 200.f)
					volume = 100.f;
				soundOutput->ChangeOutputVolume(volume / 100.f);
			}

			if (input.find("inputvol") != std::string::npos)
			{
				auto out = split(input);
				auto volume = std::stof(out[1]);
				if (volume < 0.f || volume > 200.f)
					volume = 100.f;
				soundInput->ChangeMicGain(volume / 100.f);
			}

			if (input.find("toggleinput") != std::string::npos)
			{
				auto out = split(input);
				std::stoi(out[1]) ? soundInput->EnableInput() : soundInput->DisableInput();
			}

			if (input.find("opus") != std::string::npos)
			{
				auto out = split(input);
				opusTest = std::stoi(out[1]);
			}

			if (input.find("noise") != std::string::npos)
			{
				auto out = split(input);
				soundInput->SetNoiseSuppressionStatus(std::stoi(out[1]));
			}

			if (input.find("normalize") != std::string::npos)
			{
				auto out = split(input);
				soundInput->SetNormalizationEnabled(std::stoi(out[1]));
			}

			if (input.find("inputlist") != std::string::npos)
			{
				for (auto& p : soundInput->GetInputDevices())
				{
					printf("%d: %s\n", p.first, p.second.c_str());
				}
			}

			if (input.find("outputlist") != std::string::npos)
			{
				for (auto& p : soundOutput->GetOutputDevices())
				{
					printf("%d: %s\n", p.first, p.second.c_str());
				}
			}

			if (input.find("setoutput") != std::string::npos)
			{
				auto out = split(input);
				soundOutput->SetOutputDevice(std::stoi(out[1]));
			}

			if (input.find("setinput") != std::string::npos)
			{
				auto out = split(input);
				soundInput->SetInputDevice(std::stoi(out[1]));
			}
		}
	});

	while (1)
	{
		Sleep(0);
	}
}
