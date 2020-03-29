#include <chrono>
#include <iostream>
#include <string>
#include "Platform.h"
#include "CPU.h"

int main(int argc, char** argv) {
	if (argc != 4) {
		std::cerr << "Usage: " << argv[0] << " <Scale> <Delay> <ROM>\n";
		std::exit(EXIT_FAILURE);
	}

	int videoScale = std::stoi(argv[1]);
	int cycleDelay = std::stoi(argv[2]);
	char const* romFileName = argv[3];

	Platform platform("ChipEi", cst::VIDEO_WIDTH * videoScale, cst::VIDEO_HEIGHT * videoScale, cst::VIDEO_WIDTH, cst::VIDEO_HEIGHT);
	
	CPU chip8;
	chip8.LoadROM(romFileName);

	// Flags for load, shift and dotted_rendering
	chip8.experimental.load_flag = false;
	chip8.experimental.shift_flag = false;
	chip8.experimental.dotted_rendering_flag = false;

	if (!chip8.isRomLoaded()) {
		std::exit(EXIT_FAILURE);
	}

	int videoPitch;

	auto lastCycleTime = std::chrono::high_resolution_clock::now();
	
	bool quit = false;
	while (!quit) {
		quit = platform.ProcessInput(chip8.keypad) | chip8.shouldClose();
		//platform.ProcessSound(chip8.isSoundPlaying());

		videoPitch = sizeof(chip8.current_video[0]) * cst::VIDEO_WIDTH;

		auto currentTime = std::chrono::high_resolution_clock::now();
		float dt = std::chrono::duration<float, std::chrono::milliseconds::period>(currentTime - lastCycleTime).count();

		if (dt > cycleDelay) {
			lastCycleTime = currentTime;
			chip8.Cycle();
			platform.Update(chip8.current_video, videoPitch);
		}
	}
}