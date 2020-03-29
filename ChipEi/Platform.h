#pragma once
#include <cstdint>
#include <iostream>
#include <math.h>
#include <SDL.h>
#include <SDL_audio.h>
#include "CPU.h"

const int AMPLITUDE = 28000;
const int SAMPLE_RATE = 44100;

class Platform {
public:
	Platform(char const* title, int windowWidth, int windowHeight, int textureWidth, int textureHeight);
	~Platform();
	void Update(void const* buffer, int pitch);
	bool ProcessInput(uint8_t* keys);
	void ProcessSound(bool play);
private:
	void GetAudioDevice();

	SDL_Window* window{};
	SDL_Renderer* renderer{};
	SDL_Texture* texture{};
	SDL_AudioDeviceID dev{};
};

void audio_callback(void* user_data, uint8_t* raw_buffer, int bytes);