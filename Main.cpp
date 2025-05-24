#include <iostream>
#include <string>

#include <SDL2/SDL.h>
#include <rtaudio/RtAudio.h>

using namespace std;

SDL_Window* SdlWindow;
SDL_Renderer* SdlRenderer;
SDL_Event SdlEvent;
SDL_FRect SdlFRect;

string Title = "Audio-Visualizer";
int Width = 1280;
int Height = 720;
int CX = Width / 2;
int CY = Height / 2;
int Frame;

RtAudio TheRtAudio;
int DeviceId = -1;
float* SampleBuffer;
unsigned int BufferSize = 2048;
unsigned int Samplerate = 44100;
unsigned int Channels = 1;

float* DisplayBuffer1 = new float[BufferSize];
float* DisplayBuffer2 = new float[BufferSize];
float* DisplayBuffer3 = new float[BufferSize];

int AudioCallback(void* OutputBuffer, void* InputBuffer, unsigned int BufferFrames, double StreamTime, RtAudioStreamStatus Status, void* UserData)
{
	SampleBuffer = reinterpret_cast<float*>(InputBuffer);
	return 0;
}

int main()
{
	srand(time(nullptr));

	if (DeviceId == -1)
	{
		while (true)
		{
			system("clear");
			cout << "Select Audio Device (-1 for exit)" << endl;

			int DeviceCount = TheRtAudio.getDeviceCount();
			for (int I = 0; I < DeviceCount; I++)
			{
				RtAudio::DeviceInfo TheDeviceInfo = TheRtAudio.getDeviceInfo(I);
				cout << I << ": " << TheDeviceInfo.name << endl;
			}

			cin >> DeviceId;
			if (DeviceId >= -1 && DeviceId < DeviceCount) { break; }
		}
	}

	if (DeviceId == -1) { return 0; }
	system("clear");

	int Result = SDL_Init(SDL_INIT_VIDEO);
	if (Result < 0) { throw runtime_error("Error: SDL_Init"); }

	SdlWindow = SDL_CreateWindow(Title.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, Width, Height, SDL_WINDOW_HIDDEN);
	if (SdlWindow == NULL) { throw runtime_error("Error: SDL_CreateWindow"); }

	SdlRenderer = SDL_CreateRenderer(SdlWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (SdlRenderer == NULL) { throw runtime_error("Error: SDL_CreateRenderer"); }

	RtAudio::StreamParameters TheStreamParameters;
	TheStreamParameters.deviceId = DeviceId;
	TheStreamParameters.nChannels = Channels;

	TheRtAudio.openStream(nullptr, &TheStreamParameters, RTAUDIO_FLOAT32, Samplerate, &BufferSize, &AudioCallback);
	TheRtAudio.startStream();

	for (int I = 0; I < BufferSize; I++)
	{
		DisplayBuffer1[I] = 0;
		DisplayBuffer2[I] = 0;
		DisplayBuffer3[I] = 0;
	}

	bool IsRunning = true;
	SDL_ShowWindow(SdlWindow);

	while (IsRunning)
	{
		while (SDL_PollEvent(&SdlEvent))
		{
			if (SdlEvent.type == SDL_QUIT) { IsRunning = false; }
		}

		for (int I = 0; I < BufferSize; I++)
		{
			DisplayBuffer3[I] = (DisplayBuffer3[I] + DisplayBuffer2[I]) * 0.7;
			DisplayBuffer2[I] = (DisplayBuffer2[I] + DisplayBuffer1[I]) * 0.6;
			DisplayBuffer1[I] = (DisplayBuffer1[I] + SampleBuffer[I]) * 0.5;
		}

		SDL_SetRenderDrawColor(SdlRenderer, 0, 0, 0, 255);
		SDL_RenderClear(SdlRenderer);

		for (int I = 0; I < BufferSize; I++)
		{
			float X = I * Width / (float)BufferSize;
			float W = (I + 1) * Width / (float)BufferSize - X;

			float Factor = I / (float)BufferSize;
			float FrameFactor = (Frame % 500) / 500.0f;
			Factor += FrameFactor;

			Factor = fmod(Factor, 1.0f);

			int Value1 = (int)(Factor * 6);
			float Value2 = fmod(Factor * 6, 1.0f);

			float R = 0;
			float G = 0;
			float B = 0;

			if (Value1 == 0) { R = 1; G = Value2; B = 0; }
			else if (Value1 == 1) { R = 1 - Value2; G = 1; B = 0; }
			else if (Value1 == 2) { R = 0; G = 1; B = Value2; }
			else if (Value1 == 3) { R = 0; G = 1 - Value2; B = 1; }
			else if (Value1 == 4) { R = Value2; G = 0; B = 1; }
			else if (Value1 == 5) { R = 1; G = 0; B = 1 - Value2; }

			SdlFRect.x = X;
			SdlFRect.y = CY;
			SdlFRect.w = W;
			SdlFRect.h = DisplayBuffer3[I] * CY * 0.5;
			SDL_SetRenderDrawColor(SdlRenderer, R * 255, G * 255, B * 255, 255);
			SDL_RenderFillRectF(SdlRenderer, &SdlFRect);

			SdlFRect.x = X;
			SdlFRect.y = CY;
			SdlFRect.w = W;
			SdlFRect.h = DisplayBuffer2[I] * CY * 0.4;
			SDL_SetRenderDrawColor(SdlRenderer, R * 191, G * 191, B * 191, 255);
			SDL_RenderFillRectF(SdlRenderer, &SdlFRect);

			SdlFRect.x = X;
			SdlFRect.y = CY;
			SdlFRect.w = W;
			SdlFRect.h = DisplayBuffer1[I] * CY * 0.3;
			SDL_SetRenderDrawColor(SdlRenderer, R * 127, G * 127, B * 127, 255);
			SDL_RenderFillRectF(SdlRenderer, &SdlFRect);
		}

		SDL_RenderPresent(SdlRenderer);
		Frame++;
	}

	TheRtAudio.stopStream();
	TheRtAudio.closeStream();

	SDL_DestroyWindowSurface(SdlWindow);
	SDL_DestroyWindow(SdlWindow);
	SDL_Quit();

	delete[] DisplayBuffer1;
	delete[] DisplayBuffer2;
	delete[] DisplayBuffer3;

	return 0;
}
