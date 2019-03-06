#include <alt-voice.h>

#include <cmath>
#include <sstream>
#include <thread>
#include <chrono>

IStreamPlayer* streamPlayer = nullptr;
IStreamPlayer* streamPlayer2 = nullptr;

bool streamActive1 = true;
bool streamActive2 = true;

void OnVoiceInput(const void* buffer, int length, float micLevel)
{
	if (streamPlayer && streamActive1)
		streamPlayer->PushOpusBuffer(buffer, length);

	if (streamPlayer2 && streamActive2)
		streamPlayer2->PushOpusBuffer(buffer, length);
}

float posOnCircle = 0;
const float radius = 5.f;

int main()
{
	int sampleRate = 24000;
	int framesPerBuffer = 420;
	int bitrate = 16000;

	ISoundInput* soundInput;
	CreateSoundInput(NULL, sampleRate, framesPerBuffer, bitrate, &soundInput);
	soundInput->RegisterCallback(OnVoiceInput);
	soundInput->EnableInput();
	//soundInput->ChangeMicGain(7.f);

	ISoundOutput* soundOutput;
	CreateSoundOutput(NULL, sampleRate, 32, &soundOutput);

	//OPUS Init

	streamPlayer = soundOutput->CreateStreamPlayer();
	streamPlayer->SetMaxDistance(100.f);
	streamPlayer->SetMinDistance(30.f);

	streamPlayer2 = soundOutput->CreateStreamPlayer();
	streamPlayer2->SetMaxDistance(100.f);
	streamPlayer2->SetMinDistance(30.f);

	soundOutput->SetMyPosition(0.f, 0.f, 0.f);
	soundOutput->SetMyVelocity(0.f, 0.f, 0.f);

	soundOutput->SetMyOrientationFront(0.f, 0.f, 1.f);
	soundOutput->SetMyOrientationUp(1.f, 0.f, 0.f);

	soundOutput->UpdateMe();

	std::thread([]() {
		while (true)
		{
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
			posOnCircle += 0.02f;
			float x = radius * cosf(posOnCircle);
			float y = radius * -sinf(posOnCircle);
			float z = 0;
			streamPlayer->SetPosition(x, y, z);
			streamPlayer->SetVelocity(0.0f, 0.0f, 0.0f);
			streamPlayer->SetDirection(0.0f, 0.0f, 1.0f);
			if (!streamPlayer->Update())
				return 1;
			

			x = radius * cosf(posOnCircle + 1.5f);
			y = radius * -sinf(posOnCircle + 1.5f);
			z = 0;
			streamPlayer->SetPosition(x, y, z);
			streamPlayer2->SetVelocity(0.0f, 0.0f, 0.0f);
			streamPlayer2->SetDirection(0.0f, 0.0f, 1.0f);
			if (!streamPlayer2->Update())
				return 1;
		}
	}).detach();
	
	while (1)
	{
		int key = getchar();
		if (key == '1')
			streamActive1 ^= 1;
		else if (key == '2')
			streamActive2 ^= 1;
	}
}
