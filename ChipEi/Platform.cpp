#include "Platform.h"

Platform::Platform(char const* title, int windowWidth, int windowHeight, int textureWidth, int textureHeight) {
	if ( SDL_Init(SDL_INIT_EVERYTHING) != 0 )
		std::cout << "Error : " << SDL_GetError() << std::endl;

	window = SDL_CreateWindow(title, 0, 0, windowWidth, windowHeight, SDL_WINDOW_SHOWN);
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, textureWidth, textureHeight);
	GetAudioDevice();
}

Platform::~Platform() {
	SDL_DestroyTexture(texture);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_CloseAudio();
	SDL_Quit();
}

// Get audio device
void Platform::GetAudioDevice() {
	SDL_AudioSpec want, have;

	int sample_nr = 0;

	SDL_memset(&want, 0, sizeof(want));
	want.freq = SAMPLE_RATE;
	want.format = AUDIO_F32;
	want.channels = 1;
	want.samples = 2048;
	want.callback = audio_callback;
	want.userdata = &sample_nr;

	dev = SDL_OpenAudioDevice(NULL, 0, &want, &have, SDL_AUDIO_ALLOW_FORMAT_CHANGE);
}

// Update function for Platform class
void Platform::Update(void const* buffer, int pitch) {
	SDL_UpdateTexture(texture, nullptr, buffer, pitch);
	SDL_RenderClear(renderer);
	SDL_RenderCopy(renderer, texture, nullptr, nullptr);
	SDL_RenderPresent(renderer);
}

// Check if key has been pressed or released
bool Platform::ProcessInput(uint8_t* keys) {
	bool quit = false;
	SDL_Event event;

	while (SDL_PollEvent(&event)) {
		switch (event.type) {
			case SDL_QUIT:
				quit = true;
				break;

			case SDL_KEYDOWN:
				switch (event.key.keysym.sym) {
					case SDLK_ESCAPE: quit = true; break;
					case SDLK_x: keys[0] = 1; break;
					case SDLK_1: keys[1] = 1; break;
					case SDLK_2: keys[2] = 1; break;
					case SDLK_3: keys[3] = 1; break;
					case SDLK_q: keys[4] = 1; break;
					case SDLK_w: keys[5] = 1; break;
					case SDLK_e: keys[6] = 1; break;
					case SDLK_a: keys[7] = 1; break;
					case SDLK_s: keys[8] = 1; break;
					case SDLK_d: keys[9] = 1; break;
					case SDLK_z: keys[0xA] = 1; break;
					case SDLK_c: keys[0xB] = 1; break;
					case SDLK_4: keys[0xC] = 1; break;
					case SDLK_r: keys[0xD] = 1; break;
					case SDLK_f: keys[0xE] = 1; break;
					case SDLK_v: keys[0xF] = 1; break;
				}
				break;

			case SDL_KEYUP:
				switch (event.key.keysym.sym) {
					case SDLK_x: keys[0] = 0; break;
					case SDLK_1: keys[1] = 0; break;
					case SDLK_2: keys[2] = 0; break;
					case SDLK_3: keys[3] = 0; break;
					case SDLK_q: keys[4] = 0; break;
					case SDLK_w: keys[5] = 0; break;
					case SDLK_e: keys[6] = 0; break;
					case SDLK_a: keys[7] = 0; break;
					case SDLK_s: keys[8] = 0; break;
					case SDLK_d: keys[9] = 0; break;
					case SDLK_z: keys[0xA] = 0; break;
					case SDLK_c: keys[0xB] = 0; break;
					case SDLK_4: keys[0xC] = 0; break;
					case SDLK_r: keys[0xD] = 0; break;
					case SDLK_f: keys[0xE] = 0; break;
					case SDLK_v: keys[0xF] = 0; break;
				}
				break;
		}
	}
	return quit;
}

// Play buzzer tone
void Platform::ProcessSound(bool play) {
	if (play) {
		SDL_PauseAudioDevice(dev, 0);
		SDL_Delay(1000);
		SDL_PauseAudioDevice(dev, 1);
	}
}

// Audio callback
void audio_callback(void* user_data, uint8_t* raw_buffer, int bytes) {
	std::cout << "Sound Playing" << std::endl;
	Sint16 *buffer = (Sint16*)raw_buffer;
	int length = bytes / 2;
	int &sample_nr(*(int*)user_data);

	for(int i = 0; i < length; i++, sample_nr++) {
        double time = (double)sample_nr / (double)SAMPLE_RATE;
        buffer[i] = (Sint16)(AMPLITUDE * sin(2.0f * M_PI * 441.0f * time)); // render 441 HZ sine wave
    }
}
