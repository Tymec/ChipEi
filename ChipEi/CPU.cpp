#include "CPU.h"

uint8_t fontset[cst::FONTSET_SIZE] = {
	0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
	0x20, 0x60, 0x20, 0x20, 0x70, // 1
	0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
	0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
	0x90, 0x90, 0xF0, 0x10, 0x10, // 4
	0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
	0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
	0xF0, 0x10, 0x20, 0x40, 0x40, // 7
	0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
	0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
	0xF0, 0x90, 0xF0, 0x90, 0x90, // A
	0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
	0xF0, 0x80, 0x80, 0x80, 0xF0, // C
	0xE0, 0x90, 0x90, 0x90, 0xE0, // D
	0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
	0xF0, 0x80, 0xF0, 0x80, 0x80,  // F

	// high-res mode font sprites (0-9)
    0x3C, 0x7E, 0xE7, 0xC3, 0xC3, 0xC3, 0xC3, 0xE7, 0x7E, 0x3C, 
    0x18, 0x38, 0x58, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x3C,
    0x3E, 0x7F, 0xC3, 0x06, 0x0C, 0x18, 0x30, 0x60, 0xFF, 0xFF,
    0x3C, 0x7E, 0xC3, 0x03, 0x0E, 0x0E, 0x03, 0xC3, 0x7E, 0x3C,
    0x06, 0x0E, 0x1E, 0x36, 0x66, 0xC6, 0xFF, 0xFF, 0x06, 0x06,
    0xFF, 0xFF, 0xC0, 0xC0, 0xFC, 0xFE, 0x03, 0xC3, 0x7E, 0x3C,
    0x3E, 0x7C, 0xC0, 0xC0, 0xFC, 0xFE, 0xC3, 0xC3, 0x7E, 0x3C,
    0xFF, 0xFF, 0x03, 0x06, 0x0C, 0x18, 0x30, 0x60, 0x60, 0x60,
    0x3C, 0x7E, 0xC3, 0xC3, 0x7E, 0x7E, 0xC3, 0xC3, 0x7E, 0x3C,
    0x3C, 0x7E, 0xC3, 0xC3, 0x7F, 0x3F, 0x03, 0x03, 0x3E, 0x7C
};

CPU::CPU() : randGen(std::chrono::system_clock::now().time_since_epoch().count()) {
	// Set PC to start address
	pc = cst::START_ADDRESS;

	// Set current display video memory
	current_video = video;

	// Load fonts into memory
	for (unsigned int i = 0; i < cst::FONTSET_SIZE; ++i) {
		memory[cst::FONTSET_START_ADDRESS + i] = fontset[i];
	}

	// Initialize RNG
	randByte = std::uniform_int_distribution<unsigned short int>(0, 255);
}

// Load the ROM file into memory.
void CPU::LoadROM(char const* filename) {
	// Open the file as binary and put cursor at the end
	std::ifstream file(filename, std::ios::binary | std::ios::ate);

	if (file.is_open()) {
		// Get size of the file and allocate buffer with size
		std::streampos size = file.tellg();
		char* buffer = new char[size];

		// Reset cursor position and fill file stream into buffer
		file.seekg(0, std::ios::beg);
		file.read(buffer, size);
		file.close();

		// Load the ROM into memory
		for (long i = 0; i < size; ++i) {
			memory[cst::START_ADDRESS + i] = buffer[i];
		}

		// Clear the buffer
		delete[] buffer;

		_isRomLoaded = true;
		return;
	}
	std::cout << "File failed to open." << std::endl;
}

// Cycle: Fetch, Decode, Execute
void CPU::Cycle() {
	// Fetch: XX00 + 00XX
	opcode = (memory[pc] << 8u) | memory[pc + 1];

	// Increment PC
	pc += 2;

	// Decode and Execute
	ParseOpcodes();

	// Decrement delay timer if it's been set
	if (delayTimer > 0)
		--delayTimer;

	// Decrement sound timer if it's been set
	if (soundTimer > 0)
		--soundTimer;
}

// Check if ROM is loaded
bool CPU::isRomLoaded() { return _isRomLoaded; }

// Check if sound is playing
bool CPU::isSoundPlaying() { return soundTimer > 0; }

// Check if interpreter should close
bool CPU::shouldClose() { return quit; }

// Parse opcodes
void CPU::ParseOpcodes() {
	//std::cout << "Opcode: " << std::hex << opcode << std::endl;
	
	switch(opcode & 0xF000u) {
		// Opcodes starting with $0
		case (0x0000):
			if ((opcode & 0x00F0u) == 0xB0) {
				CPU::OP_00Bn(); // SuperChip-8
				break;
			} else if ((opcode & 0x00F0u) == 0xC0) {
				CPU::OP_00Cn(); // SuperChip-8
				break;
			}

			switch (opcode & 0x00FFu) {
				case (0xE0):
					CPU::OP_00E0();
					break;
				case (0xEE):
					CPU::OP_00EE();
					break;
				case (0xFB):
					CPU::OP_00FB(); // SuperChip-8
					break;
				case (0xFC):
					CPU::OP_00FC(); // SuperChip-8
					break;
				case (0xFD):
					CPU::OP_00FD(); // SuperChip-8
					break;
				case (0xFE):
					CPU::OP_00FE(); // SuperChip-8
					break;
				case (0xFF):
					CPU::OP_00FF(); // SuperChip-8
					break;
			}
			break;
		// Opcodes starting with $1
		case (0x1000):
			CPU::OP_1nnn();
			break;
		// Opcodes starting with $2
		case (0x2000):
			CPU::OP_2nnn();
			break;
		// Opcodes starting with $3
		case (0x3000):
			CPU::OP_3xkk();
			break;
		// Opcodes starting with $4
		case (0x4000):
			CPU::OP_4xkk();
			break;
		// Opcodes starting with $5
		case (0x5000):
			CPU::OP_5xy0();
			break;
		// Opcodes starting with $6
		case (0x6000):
			CPU::OP_6xkk();
			break;
		// Opcodes starting with $7
		case (0x7000):
			CPU::OP_7xkk();
			break;
		// Opcodes starting with $8
		case (0x8000):
			switch (opcode & 0x000F) {
				case (0x0):
					CPU::OP_8xy0();
					break;
				case (0x1):
					CPU::OP_8xy1();
					break;
				case (0x2):
					CPU::OP_8xy2();
					break;
				case (0x3):
					CPU::OP_8xy3();
					break;
				case (0x4):
					CPU::OP_8xy4();
					break;
				case (0x5):
					CPU::OP_8xy5();
					break;
				case (0x6):
					CPU::OP_8xy6();
					break;
				case (0x7):
					CPU::OP_8xy7();
					break;
				case (0xE):
					CPU::OP_8xyE();
					break;
			}
			break;
		// Opcodes starting with $9
		case (0x9000):
			CPU::OP_9xy0();
			break;
		// Opcodes starting with $A
		case (0xA000):
			CPU::OP_Annn();
			break;
		// Opcodes starting with $B
		case (0xB000):
			CPU::OP_Bnnn();
			break;
		// Opcodes starting with $C
		case (0xC000):
			CPU::OP_Cxkk();
			break;
		// Opcodes starting with $D
		case (0xD000):
			if ((opcode & 0x000Fu) == 0) {
				CPU::OP_Dxy0();
			} else {
				CPU::OP_Dxyn(); // SuperChip-8
			}
			break;
		// Opcodes starting with $E
		case (0xE000):
			if ((opcode & 0x000Fu) == 0x1) {
				CPU::OP_ExA1();
			} else if ((opcode & 0x000Fu) == 0xE) {
				CPU::OP_Ex9E();
			}
			break;
		// Opcodes starting with $F
		case (0xF000):
			switch(opcode & 0x00FFu) {
				case (0x07):
					CPU::OP_Fx07();
					break;
				case (0x0A):
					CPU::OP_Fx0A();
					break;
				case (0x15):
					CPU::OP_Fx15();
					break;
				case (0x18):
					CPU::OP_Fx18();
					break;
				case (0x1E):
					CPU::OP_Fx1E();
					break;
				case (0x29):
					CPU::OP_Fx29();
					break;
				case (0x30):
					CPU::OP_Fx30(); // SuperChip-8
					break;
				case (0x33):
					CPU::OP_Fx33();
					break;
				case (0x55):
					CPU::OP_Fx55();
					break;
				case (0x65):
					CPU::OP_Fx65();
					break;
				case (0x75):
					CPU::OP_Fx75(); // SuperChip-8
					break;
				case (0x85):
					CPU::OP_Fx85(); // SuperChip-8
					break;
			}
			break;
		// Incorrect opcodes
		default:
			OP_NULL();
			break;
	}
}

// NULL: Opcodes that are incorrect
void CPU::OP_NULL() {
	std::cout << "Incorrect Opcode: " << std::hex << opcode << std::endl;
}

/// Chip-8

// CLS: Clear the display.
//
void CPU::OP_00E0() {
	//std::fill_n(current_video, sizeof(current_video), 0);
	memset(current_video, 0, sizeof(current_video));
}

// RET: Return from a subroutine.
// The interpreter sets the program counter to the address at the top of the stack, then subtracts 1 from the stack pointer.
void CPU::OP_00EE() {
	--sp;
	pc = stack[sp];
}

// COMPAT: Non-standard. Toggles changing of the I register by save (FX55) and restore (FX65) opcodes.
//
void CPU::OP_00FA() {
	experimental.load_flag = !experimental.load_flag;
}

// JP addr: Jump to location nnn.
// The interpreter sets the program counter to nnn.
void CPU::OP_1nnn() {
	uint16_t address = opcode & 0x0FFFu;
	pc = address;
}

// CALL addr: Call subroutine at nnn.
// The interpreter increments the stack pointer, then puts the current PC on the top of the stack. The PC is then set to nnn.
void CPU::OP_2nnn() {
	uint16_t address = opcode & 0x0FFFu;
	stack[sp] = pc;
	++sp;
	pc = address;
}

// SE Vx, byte: Skip next instruction if Vx = kk.
// The interpreter compares register Vx to kk, and if they are equal, increments the program counter by 2.
void CPU::OP_3xkk() {
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t byte = opcode & 0x00FFu;

	if (registers[Vx] == byte)
		pc += 2;
}

// SNE Vx, byte: Skip next instruction if Vx != kk.
// The interpreter compares register Vx to kk, and if they are not equal, increments the program counter by 2.
void CPU::OP_4xkk() {
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t byte = opcode & 0x00FFu;

	if (registers[Vx] != byte)
		pc += 2;
}

// SE Vx, Vy: Skip next instruction if Vx = Vy.
// The interpreter compares register Vx to register Vy, and if they are equal, increments the program counter by 2.
void CPU::OP_5xy0() {
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	if (registers[Vx] == registers[Vy])
		pc += 2;
}

// LD Vx, byte: Set Vx = kk.
// The interpreter puts the value kk into register Vx.
void CPU::OP_6xkk() {
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t byte = opcode & 0x00FFu;

	registers[Vx] = byte;
}

// ADD Vx, byte: Set Vx = Vx + kk.
// Adds the value kk to the value of register Vx, then stores the result in Vx. 
void CPU::OP_7xkk() {
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t byte = opcode & 0x00FFu;

	registers[Vx] += byte;
}

// LD Vx, Vy: Set Vx = Vy.
// Stores the value of register Vy in register Vx.
void CPU::OP_8xy0() {
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	registers[Vx] = registers[Vy];
}

// OR Vx, Vy: Set Vx = Vx OR Vy.
// Performs a bitwise OR on the values of Vx and Vy, then stores the result in Vx. 
// A bitwise OR compares the corrseponding bits from two values, and if either bit is 1, 
// then the same bit in the result is also 1. Otherwise, it is 0. 
void CPU::OP_8xy1() {
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	registers[Vx] |= registers[Vy];
}

// AND Vx, Vy: Set Vx = Vx AND Vy.
// Performs a bitwise AND on the values of Vx and Vy, then stores the result in Vx. 
// A bitwise AND compares the corrseponding bits from two values, and if both bits are 1, 
// then the same bit in the result is also 1. Otherwise, it is 0. 
void CPU::OP_8xy2() {
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	registers[Vx] &= registers[Vy];
}

// XOR Vx, Vy: Set Vx = Vx XOR Vy.
// Performs a bitwise exclusive OR on the values of Vx and Vy, then stores the result in Vx. 
// An exclusive OR compares the corrseponding bits from two values, and if the bits are not both the same,
// then the corresponding bit in the result is set to 1. Otherwise, it is 0.
void CPU::OP_8xy3() {
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	registers[Vx] ^= registers[Vy];
}

// ADD Vx, Vy: Set Vx = Vx + Vy, set VF = carry.
// The values of Vx and Vy are added together. If the result is greater than 8 bits (i.e., > 255,) VF is set to 1, otherwise 0. 
// Only the lowest 8 bits of the result are kept, and stored in Vx.
void CPU::OP_8xy4() {
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	uint8_t sum = registers[Vx] + registers[Vy];

	registers[cst::VF] = sum > 255u ? 1 : 0;
	registers[Vx] = sum & 0xFFu;
}

// SUB Vx, Vy: Set Vx = Vx - Vy, set VF = NOT borrow.
// If Vx > Vy, then VF is set to 1, otherwise 0. Then Vy is subtracted from Vx, and the results stored in Vx.
void CPU::OP_8xy5() {
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	registers[cst::VF] = registers[Vx] > registers[Vy] ? 1 : 0;
	registers[Vx] -= registers[Vy];
}

// SHR Vx: Set Vx = Vx SHR 1.
// If the least-significant bit of Vx is 1, then VF is set to 1, otherwise 0. Then Vx is divided by 2.
void CPU::OP_8xy6() {
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vc = Vx;

	if (experimental.shift_flag) {
		uint8_t Vy = (opcode & 0x00F0u) >> 4u;
		Vc = Vy;
	}

	// Store LSB in VF
	registers[cst::VF] = (registers[Vc] & 0x1u);

	registers[Vx] = registers[Vc] >> 1;
	//registers[Vx] >>= 1;
}

// SUBN Vx, Vy: Set Vx = Vy - Vx, set VF = NOT borrow.
// If Vy > Vx, then VF is set to 1, otherwise 0. Then Vx is subtracted from Vy, and the results stored in Vx.
void CPU::OP_8xy7() {
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	registers[cst::VF] = registers[Vy] > registers[Vx] ? 1 : 0;
	registers[Vx] = registers[Vy] - registers[Vx];
}

// SHL Vx {, Vy}: Set Vx = Vx SHL 1.
// If the most-significant bit of Vx is 1, then VF is set to 1, otherwise to 0. Then Vx is multiplied by 2.
void CPU::OP_8xyE() {
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vc = Vx;

	if (experimental.shift_flag) {
		uint8_t Vy = (opcode & 0x00F0u) >> 4u;
		Vc = Vy;
	}

	// Store MSB in VF
	registers[cst::VF] = (registers[Vc] & 0x80u) >> 7u;

	registers[Vx] = registers[Vc] << 1;
	//registers[Vx] <<= 1;
}

// SNE Vx, Vy: Skip next instruction if Vx != Vy.
// The values of Vx and Vy are compared, and if they are not equal, the program counter is increased by 2.
void CPU::OP_9xy0() {
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	if (registers[Vx] != registers[Vy])
		pc += 2;
}

// LD I, addr: Set I = nnn.
// The value of register I is set to nnn.
void CPU::OP_Annn() {
	uint16_t address = opcode & 0x0FFFu;
	index = address;
}

// JP V0, addr: Jump to location nnn + V0.
// The program counter is set to nnn plus the value of V0.
void CPU::OP_Bnnn() {
	uint16_t address = opcode & 0x0FFFu;
	pc = address + registers[cst::V0];
}

// RND Vx, byte: Set Vx = random byte AND kk.
// The interpreter generates a random number from 0 to 255, which is then ANDed with the value kk. 
// The results are stored in Vx. See instruction 8xy2 for more information on AND.
void CPU::OP_Cxkk() {
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t byte = opcode & 0x00FFu;

	registers[Vx] = randByte(randGen) & byte;
}

// DRW Vx, Vy, nibble: Display n-byte sprite starting at memory location I at (Vx, Vy), set VF = collision.
// The interpreter reads n bytes from memory, starting at the address stored in I. 
// These bytes are then displayed as sprites on screen at coordinates (Vx, Vy). Sprites are XORed onto the existing screen. 
// If this causes any pixels to be erased, VF is set to 1, otherwise it is set to 0. 
// If the sprite is positioned so part of it is outside the coordinates of the display, it wraps around to the opposite side of the screen. 
void CPU::OP_Dxyn() {
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;
	uint8_t height = opcode & 0x000Fu;

	// Wrap if going beyond screen bounds
	uint8_t xPos = (registers[Vx] * (2 - extendedMode)) % cst::VIDEO_WIDTH; // multiply with 2 if not in extended mode 
	uint8_t yPos = (registers[Vy] * (2 - extendedMode)) % cst::VIDEO_HEIGHT; // multiply with 2 if not in extended mode 

	// Reset VF
	registers[cst::VF] = 0;

	uint8_t sprite_size = cst::SPRITE_SIZE;

	for (unsigned int row = 0; row < height; ++row) {
		uint8_t spriteByte = memory[index + row];
		for (unsigned int col = 0; col < sprite_size; ++col) {
			uint8_t nxPos = xPos + (col * (2 - extendedMode)); 
			uint8_t nyPos = yPos + (row * (2 - extendedMode));
			nxPos = nxPos % cst::VIDEO_WIDTH; 
			nyPos = nyPos % cst::VIDEO_HEIGHT;

			uint8_t spritePixel = spriteByte & (0x80u >> col);

			if (spritePixel) {
				if (!extendedMode || experimental.dotted_rendering_flag) {
					// If not in extended mode, 1x1 sprite pixel == 2x2 screen pixels
					uint32_t* screenPixel1 = &current_video[nyPos * cst::VIDEO_WIDTH + nxPos];
					uint32_t* screenPixel2 = &current_video[(nyPos + 1) * cst::VIDEO_WIDTH + nxPos];
					uint32_t* screenPixel3 = &current_video[(nyPos + 1) * cst::VIDEO_WIDTH + (nxPos + 1)];
					uint32_t* screenPixel4 = &current_video[nyPos * cst::VIDEO_WIDTH + (nxPos + 1)];
					registers[cst::VF] = (*screenPixel1 == 0xFFFFFFFF) | (*screenPixel2 == 0xFFFFFFFF) | (*screenPixel3 == 0xFFFFFFFF) | (*screenPixel4 == 0xFFFFFFFF);

					*screenPixel1 ^= 0xFFFFFFFF;
					*screenPixel2 ^= 0xFFFFFFFF;
					*screenPixel3 ^= 0xFFFFFFFF;
					*screenPixel4 ^= 0xFFFFFFFF;
				} else {
					uint32_t* screenPixel = &current_video[nyPos * cst::VIDEO_WIDTH + nxPos];
					registers[cst::VF] = *screenPixel == 0xFFFFFFFF;

					*screenPixel ^= 0xFFFFFFFF;
				}
			}
		}
	}
	/*
	// Loop over each row and column of the sprite
	for (unsigned int row = 0; row < height; ++row) {
		uint8_t spriteByte = memory[index + row];

		for (unsigned int col = 0; col < sprite_size; ++col) {
			uint8_t nxPos = xPos + col; 
			uint8_t nyPos = yPos + row; 
			nxPos = nxPos % cst::VIDEO_WIDTH; 
			nyPos = nyPos % cst::VIDEO_HEIGHT;

			uint8_t spritePixel = spriteByte & (0x80u >> col);
			uint32_t* screenPixel = &current_video[nyPos * cst::VIDEO_WIDTH + nxPos];

			// If sprite pixel is on
			if (spritePixel) {
				// Collision
				if (*screenPixel == 0xFFFFFFFF) {
					registers[cst::VF] = 1;
				}

				*screenPixel ^= 0xFFFFFFFF;
			}
		}
	}
	*/
}

// SKP Vx: Skip next instruction if key with the value of Vx is pressed.
// Checks the keyboard, and if the key corresponding to the value of Vx is currently in the down position, PC is increased by 2.
void CPU::OP_Ex9E() {
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t key = registers[Vx];

	if (keypad[key])
		pc += 2;
}

// SKNP Vx: Skip next instruction if key with the value of Vx is not pressed.
// Checks the keyboard, and if the key corresponding to the value of Vx is currently in the up position, PC is increased by 2.
void CPU::OP_ExA1() {
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t key = registers[Vx];

	if (!keypad[key])
		pc += 2;

}

// LD Vx, DT: Set Vx = delay timer value.
// The value of DT is placed into Vx.
void CPU::OP_Fx07() {
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	registers[Vx] = delayTimer;
}

// LD Vx, K: Wait for a key press, store the value of the key in Vx.
// All execution stops until a key is pressed, then the value of that key is stored in Vx.
void CPU::OP_Fx0A() {
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t key = registers[Vx];

	/*
	if (keypad[0])
	{
		registers[Vx] = 0;
	}
	else if (keypad[1])
	{
		registers[Vx] = 1;
	}
	else if (keypad[2])
	{
		registers[Vx] = 2;
	}
	else if (keypad[3])
	{
		registers[Vx] = 3;
	}
	else if (keypad[4])
	{
		registers[Vx] = 4;
	}
	else if (keypad[5])
	{
		registers[Vx] = 5;
	}
	else if (keypad[6])
	{
		registers[Vx] = 6;
	}
	else if (keypad[7])
	{
		registers[Vx] = 7;
	}
	else if (keypad[8])
	{
		registers[Vx] = 8;
	}
	else if (keypad[9])
	{
		registers[Vx] = 9;
	}
	else if (keypad[10])
	{
		registers[Vx] = 10;
	}
	else if (keypad[11])
	{
		registers[Vx] = 11;
	}
	else if (keypad[12])
	{
		registers[Vx] = 12;
	}
	else if (keypad[13])
	{
		registers[Vx] = 13;
	}
	else if (keypad[14])
	{
		registers[Vx] = 14;
	}
	else if (keypad[15])
	{
		registers[Vx] = 15;
	}
	else
	{
		pc -= 2;
	}
	*/

	for (uint8_t i = 0; i < 16; i++) {
		if (keypad[i]) {
			registers[Vx] = i;
			return;
		}
	}
	pc -= 2;
}

// LD DT, Vx: Set delay timer = Vx.
// DT is set equal to the value of Vx.
void CPU::OP_Fx15() {
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	delayTimer = registers[Vx];
}

// LD ST, Vx: Set sound timer = Vx.
// ST is set equal to the value of Vx.
void CPU::OP_Fx18() {
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	soundTimer = registers[Vx];
}

// ADD I, Vx: Set I = I + Vx.
// The values of I and Vx are added, and the results are stored in I.
void CPU::OP_Fx1E() {
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	index += registers[Vx];
}

// LD F, Vx: Set I = location of sprite for digit Vx.
// The value of I is set to the location for the hexadecimal sprite corresponding to the value of Vx.
void CPU::OP_Fx29() {
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	uint8_t digit = registers[Vx];
	index = cst::FONTSET_START_ADDRESS + (5 * digit);
}

// LD B, Vx: Store BCD representation of Vx in memory locations I, I+1, and I+2.
// The interpreter takes the decimal value of Vx, and places the hundreds digit in memory at location in I, 
// the tens digit at location I+1, and the ones digit at location I+2.
void CPU::OP_Fx33() {
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	uint8_t value = registers[Vx];

	// Ones-place
	memory[index + 2] = value % 10;
	value /= 10;

	// Tens-place
	memory[index + 1] = value % 10;
	value /= 10;

	// Hundreds-place
	memory[index] = value % 10;
}

// LD [I], Vx: Store registers V0 through Vx in memory starting at location I.
// The interpreter copies the values of registers V0 through Vx into memory, starting at the address in I.
void CPU::OP_Fx55() {
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	for (uint8_t i = 0; i <= Vx; ++i) {
		memory[index + i] = registers[i];
	}
	if (experimental.load_flag) {
		this->index += Vx + 1;
	}
}

// LD Vx, [I]: Read registers V0 through Vx from memory starting at location I.
// The interpreter reads values from memory starting at location I into registers V0 through Vx.
void CPU::OP_Fx65() {
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	for (uint8_t i = 0; i <= Vx; ++i) {
		registers[i] = memory[index + i];
	}

	if (experimental.load_flag) {
		this->index += Vx + 1;
	}
}

/// SChip-8

// SCU N: Scroll display N lines up.
//
void CPU::OP_00Bn() {
	uint8_t Vn = opcode & 0x000Fu;

	for (int row = 0; row <= Vn; row++){
        for (int col = 0; col < 128; col++){
            current_video[row * cst::VIDEO_WIDTH + col] = current_video[(row + Vn) * cst::VIDEO_WIDTH + col];
        }
    }

    for (int row = 0; row < 128; row++){
        for(int col = 0; col < Vn; col++){
            current_video[row * cst::VIDEO_WIDTH + col] = 0;
        }
    }
}

// SCD N: Scroll display N lines down.
//
void CPU::OP_00Cn() {
	uint8_t Vn = opcode & 0x000Fu;

	for (int row = 63; row >= Vn; row--){
        for (int col = 0; col < 128; col++){
            current_video[row * cst::VIDEO_WIDTH + col] = current_video[(row - Vn) * cst::VIDEO_WIDTH + col];
        }
    }

    for (int row = 0; row < 128; row++){
        for(int col = 0; col < Vn; col++){
            current_video[row * cst::VIDEO_WIDTH + col] = 0;
        }
    }
}

// SCR: Scroll display 4 pixels to the right.
//
void CPU::OP_00FB() {
	// hmmm
}

// SCL: Scroll display 4 pixels to the left.
//
void CPU::OP_00FC() {
	// hmmm
}

// EXIT: Exit the interpreter.
//
void CPU::OP_00FD() {
	quit = true;
}

// LOW: Enable low res (64x32) mode.
//
void CPU::OP_00FE() {
	extendedMode = false;
}

// HIGH: Enable high res (128x64) mode.
//
void CPU::OP_00FF() {
	extendedMode = true;
}

// DRW VX, VX, 0: When in high res mode show a 16x16 sprite at (VX, VY).
//
void CPU::OP_Dxy0() {
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	if (!extendedMode)
		return;

	// Wrap if going beyond screen bounds
	uint8_t xPos = registers[Vx] % cst::VIDEO_WIDTH;
	uint8_t yPos = registers[Vy] % cst::VIDEO_HEIGHT;

	// Reset VF
	registers[cst::VF] = 0;

	uint8_t sprite_size = cst::SPRITE_SIZE * 2;

	// Loop over each row and column of the sprite
	for (unsigned int row = 0; row < sprite_size; ++row) {
		uint8_t spriteByte = memory[index + row];

		for (unsigned int col = 0; col < sprite_size; ++col) {
			uint8_t spritePixel = spriteByte & (0x80u >> col);
			uint32_t* screenPixel = &current_video[(yPos + row) * cst::VIDEO_WIDTH + (xPos + col)];

			// If sprite pixel is on
			if (spritePixel) {
				// Collision
				if (*screenPixel == 0xFFFFFFFF) {
					registers[cst::VF] = 1;
				}

				*screenPixel ^= 0xFFFFFFFF;
			}
		}
	}
}

// LD I, FONT(VX): Set I to the address of the SCHIP-8 16x10 font sprite representing the value in VX.
//
void CPU::OP_Fx30() {
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	uint8_t digit = registers[Vx];
	index = cst::FONTSET_START_ADDRESS + (10 * digit);
}

// LD R, VX: Store V0 through VX to HP-48 RPL user flags (X <= 7).
//
void CPU::OP_Fx75() {
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	for (uint8_t i = 0; i <= Vx; ++i) {
		userRegisters[index + i] = registers[i];
	}
}

// LD VX, R: Read V0 through VX to HP-48 RPL user flags (X <= 7)
//
void CPU::OP_Fx85() {
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	for (uint8_t i = 0; i <= Vx; ++i) {
		registers[i] = userRegisters[index + i];
	}
}

/// Chip8-X

// STEPCOL: Steps background 1 color (-> blue -> black -> green -> red ->)
//
void CPU::OP_02A0() {
	background_color += 1;
	if (background_color == 4)
		background_color = 0;
}

// ADD VX, VY: Let VX = VX + VY (hex digits 00 to 77) 
// (useful for manipulating the NH, NV parameters for low resolution color.)
void CPU::OP_5xy1() {
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	// Set Color
}

// COL VX, VY: Set VY color at VX(NH), VX+1(NV) 
// (provides low resolution color 8x8.)
void CPU::OP_Bxy0() {
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	// Set Color
}

// COL VX, VY, N: N != 0, set VY color at VX, VX+1 byte N bytes vertically 
// (provides high resolution 8x32.)
void CPU::OP_Bxyn() {
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;
	uint8_t Vn = opcode & 0x000Fu;

	// Set Color
}

// SKP2 VX: Skip the following instruction if the key represented by the value in VX is pressed on hex keyboard 2.
//
void CPU::OP_ExF2() {
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
}

// SKNP2 VX: Skip the following instruction if the key represented by the value in VX is not pressed on hex keyboard 2.
//
void CPU::OP_ExF5() {
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
}

// IN VX: Copy contents from input port to VX. (Waits for EF4=1)
//
void CPU::OP_FxFB() {
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
}

// OUT VX: Output contents of VX to output port. Used to program simple sound.
//
void CPU::OP_FxF8() {
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
}

// SGT VX, VY: Skip the next instruction if register VX is greater than VY.
//
void CPU::OP_5xy1_E() {
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	if (registers[Vx] > registers[Vy])
		pc += 2;
}

// SLT VX, VY: Skip the next instruction if register VX is less than VY.
//
void CPU::OP_5xy2() {
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	if (registers[Vx] < registers[Vy])
		pc += 2;
}

// SNE VX, VY: Skip the next instruction if register VX does not equal VY.
//
void CPU::OP_5xy3() {
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	if (registers[Vx] != registers[Vy])
		pc += 2;
}

// MUL VX, VY: Set VF, VX equal to VX multipled by VY where VF is the most significant byte of a 16bit word.
//
void CPU::OP_9xy1() {
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;
}

// DIV VX, VY: Set VX equal to VX divided by VY. VF is set to the remainder.
//
void CPU::OP_9xy2() {
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	registers[Vx] = registers[Vx] / registers[Vy];
	registers[cst::VF] = registers[Vx] % registers[Vy];
}

// BCD VX, VY: Let VX, VY be treated as a 16bit word with VX the most significant part. 
// Convert that word to BCD and store the 5 digits at memory location I through I+4. I does not change.
void CPU::OP_9xy3() {
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	uint16_t value = (registers[Vx] << 8u) | registers[Vy];

	// Ones-place
	memory[index + 4] = value % 10;
	value /= 10;

	// Tens-place
	memory[index + 3] = value % 10;
	value /= 10;

	// Hundreds-place
	memory[index + 2] = value % 10;
	value /= 10;

	// Thousands-place
	memory[index + 1] = value % 10;
	value /= 10;

	// Ten-thousands-place
	memory[index] = value % 10;
}

// DISP VX: Display the value of VX on the COSMAC Elf hex display.
//
void CPU::OP_Fx75_E() {
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	// Display value of Vx in UI
}

// LD I, VX: Load I with the address of the font sprite of the ASCII value found in VX.
//
void CPU::OP_Fx94() {
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	index = registers[Vx];
}
