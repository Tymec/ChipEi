#pragma once
#include <cstdint>
#include <fstream>
#include <chrono>
#include <iostream>
#include <random>

namespace cst {
	const unsigned int FONTSET_SIZE = 240; // Fontset Size
	const unsigned int START_ADDRESS = 0x200; // PC Start Address
	const unsigned int FONTSET_START_ADDRESS = 0x50; // Fontset Start Address
	const unsigned int VIDEO_WIDTH = 128;
	const unsigned int VIDEO_HEIGHT = 64;
	const unsigned int MEMORY_SIZE = 4096;
	const unsigned int REGISTER_COUNT = 16;
	const unsigned int USER_RESISTER_COUNT = 8;
	const unsigned int STACK_LEVELS = 16;
	const unsigned int KEY_COUNT = 16;
	const unsigned int SPRITE_SIZE = 8;
	const uint8_t VF = 0xF;
	const uint8_t V0 = 0;
};

struct Experimental {
	bool dotted_rendering_flag = false;
	bool load_flag = false;
	bool shift_flag = false;
};

class CPU {
public:
	CPU();
	void LoadROM(char const* filename);
	void Cycle();
	bool isRomLoaded();
	bool isSoundPlaying();
	bool shouldClose();

	Experimental experimental{};
	uint8_t keypad[cst::KEY_COUNT]{}; // 16 Input Keys
	uint32_t *current_video;
private:
	void ParseOpcodes();

	void OP_NULL();

	// Chip-8 Instructions
	void OP_00E0();
	void OP_00EE();
	void OP_00FA();
	void OP_1nnn();
	void OP_2nnn();
	void OP_3xkk();
	void OP_4xkk();
	void OP_5xy0();
	void OP_6xkk();
	void OP_7xkk();
	void OP_8xy0();
	void OP_8xy1();
	void OP_8xy2();
	void OP_8xy3();
	void OP_8xy4();
	void OP_8xy5();
	void OP_8xy6();
	void OP_8xy7();
	void OP_8xyE();
	void OP_9xy0();
	void OP_Annn();
	void OP_Bnnn();
	void OP_Cxkk();
	void OP_Dxyn();
	void OP_Ex9E();
	void OP_ExA1();
	void OP_Fx07();
	void OP_Fx0A();
	void OP_Fx15();
	void OP_Fx18();
	void OP_Fx1E();
	void OP_Fx29();
	void OP_Fx33();
	void OP_Fx55();
	void OP_Fx65();

	// Chip-48
	void OP_Bxnn(); // Replace OP_Bnnn

	// Super Chip-8 Instructions
	void OP_00Bn();
	void OP_00Cn();
	void OP_00FB();
	void OP_00FC();
	void OP_00FD();
	void OP_00FE();
	void OP_00FF();
	void OP_Dxy0();
	void OP_Fx30();
	void OP_Fx75();
	void OP_Fx85();

	// Chip-8X Instructions
	void OP_02A0();
	void OP_5xy1();
	void OP_Bxy0();
	void OP_Bxyn();
	void OP_ExF2();
	void OP_ExF5();
	void OP_FxFB();
	void OP_FxF8();

	// Chip-8E Instructions
	void OP_5xy1_E();
	void OP_5xy2();
	void OP_5xy3();
	void OP_9xy1();
	void OP_9xy2();
	void OP_9xy3();
	void OP_Fx75_E();
	void OP_Fx94();

	uint8_t registers[cst::REGISTER_COUNT]{}; // 16 8-bit Registers
	uint8_t userRegisters[cst::USER_RESISTER_COUNT]{}; // 8 8-bit HP-RPL User Flags
	uint8_t memory[cst::MEMORY_SIZE]{}; // 4K Bytes of Memory
	uint16_t index{}; // 16-bit Index Register
	uint16_t pc{}; // 16-bit Program Counter
	uint16_t stack[cst::STACK_LEVELS]{}; // 16-level Stack
	uint8_t sp{}; // 8-bit Stack Pointer
	uint8_t delayTimer{}; // 8-bit Delay Timer
	uint8_t soundTimer{}; // 8-bit Sound Timer
	uint16_t opcode; // Current Opcode
	uint32_t video[cst::VIDEO_WIDTH * cst::VIDEO_HEIGHT]{}; // 64x32 Monochrome Display Memory (128x64 in Extended Mode)
	
	uint8_t background_color = 0;
	bool extendedMode = false;
	bool _isRomLoaded = false;
	bool quit = false;

	std::default_random_engine randGen;
	std::uniform_int_distribution<unsigned short int> randByte;
};