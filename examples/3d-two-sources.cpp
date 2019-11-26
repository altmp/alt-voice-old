#include <alt-voice.h>

#include <cmath>
#include <sstream>
#include <thread>
#include <chrono>
#include <fstream>

//IStreamPlayer* streamPlayer = nullptr;
//IStreamPlayer* streamPlayer2 = nullptr;

bool streamActive1 = true;
bool streamActive2 = true;

inline uint64_t Now()
{
	std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
	auto duration = now.time_since_epoch();
	auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
	return millis;
}

void OnVoiceInput(const void* buffer, int length, float micLevel)
{
	static int bytesWritten = 0;
	static bool writeFile = true;
	static std::ofstream out("./output.pcm", std::ofstream::binary);
	static uint64_t startTime = Now();

	if (bytesWritten < (48000 * 2 * 10) && writeFile)
	{
		out.write((char*)buffer, length);
		bytesWritten += length;
		printf("%.2f. %d bytes received\n", (float)bytesWritten / (48000 * 2 * 10), length);
	}
	else if(writeFile)
	{
		printf("Time: %d\n", (Now() - startTime));
		out.close();
		writeFile = false;
	}
	/*if (streamPlayer && streamActive1)
		streamPlayer->PushOpusBuffer(buffer, length);

	if (streamPlayer2 && streamActive2)
		streamPlayer2->PushOpusBuffer(buffer, length);*/
}

float posOnCircle = 0;
const float radius = 5.f;

int main()
{
	int sampleRate = 48000;
	int framesPerBuffer = 420;
	int bitrate = 64000;

	ISoundInput* soundInput;
	CreateSoundInput(NULL, sampleRate, framesPerBuffer, bitrate, &soundInput);
	soundInput->SetNoiseSuppressionStatus(true);
	soundInput->ChangeMicGain(2.f);
	soundInput->RegisterRawCallback(OnVoiceInput);
	soundInput->EnableInput();

	/*ISoundOutput* soundOutput;
	CreateSoundOutput(NULL, sampleRate, 32, &soundOutput);*/

	//OPUS Init

	/*streamPlayer = soundOutput->CreateStreamPlayer();
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
	}).detach();*/
	
	while (1)
	{
		int key = getchar();
		if (key == '1')
		{
			streamActive1 ^= 1;
			if (streamActive1)
				soundInput->EnableInput();
			else
				soundInput->DisableInput();
		}
		else if (key == '2')
			streamActive2 ^= 1;
	}
}
