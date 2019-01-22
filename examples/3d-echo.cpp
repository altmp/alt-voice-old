#include <alt-voice.h>
#include <Windows.h>
#include <cmath>
#include <sstream>

IStreamPlayer* streamPlayer = nullptr;
IStreamPlayer* streamPlayer2 = nullptr;

void OnVoiceInput(const void* buffer, int length, float micLevel)
{
	if (streamPlayer)
		streamPlayer->PushOpusBuffer(buffer, length);

	if (streamPlayer2)
		streamPlayer2->PushOpusBuffer(buffer, length);

#ifdef _WIN32
	std::stringstream ss;
	int symbs = micLevel * 60;
	for (int i = 0; i < symbs; ++i)
		ss << ":";
	SetConsoleTitleA(ss.str().c_str());
#endif
}

int main()
{
	int sampleRate = 24000;
	int framesPerBuffer = 120;
	int bitrate = 16000;

	ISoundInput* soundInput = CreateSoundInput(sampleRate, framesPerBuffer, bitrate);
	soundInput->RegisterCallback(OnVoiceInput);
	soundInput->EnableInput();
	//soundInput->ChangeMicGain(7.f);

	I3DSoundOutput* soundOutput = CreateSoundOutput(sampleRate);

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

	float posOnCircle = 0;
	const float radius = 5.f;

	while (true)
	{
#ifdef _WIN32
		Sleep(20);
#else
		usleep(20000);
#endif
		// posOnCircle += 0.02f;
		// float x = radius * cosf(posOnCircle);
		// float y = radius * -sinf(posOnCircle);
		// float z = 0;
		//streamPlayer->SetPosition(x, y, z);
		streamPlayer->SetPosition(5, 0, 0);
		streamPlayer->SetVelocity(0.0f, 0.0f, 0.0f);
		streamPlayer->SetDirection(0.0f, 0.0f, 1.0f);

		//streamPlayer2->SetPosition(-x, -y, z);
		streamPlayer2->SetPosition(-5, 0, 0);
		streamPlayer2->SetVelocity(0.0f, 0.0f, 0.0f);
		streamPlayer2->SetDirection(0.0f, 0.0f, 1.0f);
		//std::cout << x << ", " << y << ", " << z << std::endl;

		if (!streamPlayer->Update())
			return 1;
		if (!streamPlayer2->Update())
			return 1;
	}
	//Sound playback

}
