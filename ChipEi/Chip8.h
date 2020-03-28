#pragma once
#include <cstdint>
#include <fstream>
#include <chrono>
#include <random>

class Chip8 {
public:
	Chip8();
	void LoadROM(char const* filename);
	void Cycle();

	uint8_t registers[16]{}; // 16 8-bit Registers
	uint8_t memory[4096]{}; // 4K Bytes of Memory
	uint16_t index{}; // 16-bit Index Register
	uint16_t pc{}; // 16-bit Program Counter
	uint16_t stack[16]{}; // 16-level Stack
	uint8_t sp{}; // 8-bit Stack Pointer
	uint8_t delayTimer{}; // 8-bit Delay Timer
	uint8_t soundTimer{}; // 8-bit Sound Timer
	uint8_t keypad[16]{}; // 16 Input Keys
	uint32_t video[64 * 32]{}; // 64x32 Monochrome Display Memory
	uint16_t opcode; // Current Opcode
private:
	void SetupFunctionPointerTable();

	void Table0();
	void Table8();
	void TableE();
	void TableF();
	void OP_NULL();

	typedef void (Chip8::*Chip8Func)();
	Chip8Func table[0xF + 1]{&Chip8::OP_NULL};
	Chip8Func table0[0xE + 1]{&Chip8::OP_NULL};
	Chip8Func table8[0xE + 1]{&Chip8::OP_NULL};
	Chip8Func tableE[0xE + 1]{&Chip8::OP_NULL};
	Chip8Func tableF[0x65 + 1]{&Chip8::OP_NULL};

	void OP_00E0();
	void OP_00EE();
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

	std::default_random_engine randGen;
	std::uniform_int_distribution<uint8_t> randByte;
};

namespace cst {
	const unsigned int FONTSET_SIZE = 80; // Fontset Size
	const unsigned int START_ADDRESS = 0x200; // PC Start Address
	const unsigned int FONTSET_START_ADDRESS = 0x50; // Fontset Start Address
	const unsigned int VIDEO_WIDTH = 64;
	const unsigned int VIDEO_HEIGHT = 32;
	const unsigned int SPRITE_SIZE = 8;
	const uint8_t VF = 0xF;
	const uint8_t V0 = 0;
}
