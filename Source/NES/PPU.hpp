/**
 * Copyright (C) 2020 by Matthew Edgmon
 * matthewedgmon@gmail.com
 *
 * This file is part of mattNES.
 *
 * mattNES is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * mattNES is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with mattNES.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __PPU_HPP__
#define __PPU_HPP__

#include <bitset>
#include <cstdint>
#include <vector>

#define NES_SCREEN_WIDTH  256
#define NES_SCREEN_HEIGHT 240

#define PPU_CTRL_NAMETABLE_SELECT1  0
#define PPU_CTRL_NAMETABLE_SELECT2  1
#define PPU_CTRL_INCREMENT_MODE     2
#define PPU_CTRL_SPRITE_TILE_SELECT 3
#define PPU_CTRL_BACKG_TILE_SELECT  4
#define PPU_CTRL_SPRITE_HEIGHT      5
#define PPU_CTRL_PPU_MASTER         6
#define PPU_CTRL_NMI_ENABLE         7

#define PPU_MASK_GREYSCALE          0
#define PPU_MASK_BACKGROUND_LEFT_COLUMN_ENABLE 1
#define PPU_MASK_SPRITE_LEFT_COLUMN_ENABLE 2
#define PPU_MASK_SHOW_BACKGROUND    3
#define PPU_MASK_SHOW_SPRITES       4
#define PPU_MASK_EMPHASIZE_RED      5
#define PPU_MASK_EMPHASIZE_GREEN    6
#define PPU_MASK_EMPHASIZE_BLUE     7

#define PPU_STATUS_SPRITE_OVERFLOW  5
#define PPU_STATUS_SPRITE_0_HIT     6
#define PPU_STATUS_VBLANK           7

class NESSystem;

/**
 * PPU TODO LIST:
 *
 * Get Palletes of different PPU versions.
 * Instead of displaying a flat pallete that looks too clean, emulate NTSC signal and decode it to emulate NES video.
 *
 */

class PPU {

	public:
		PPU(NESSystem* nes_system);
		~PPU();

		void Initialize();
		void Shutdown();
		void Reset(bool hard);
		void Step();

		/* Used by CPU during OAM DMA. */
		void WriteOAM(uint8_t value);

		/* I/O functions located in PPU_IO.cpp ----------------------------------------------------------- */

		uint8_t ReadPPU(uint16_t address);              /* Internal reads from the PPU are routed here. */
		void WritePPU(uint16_t address, uint8_t value); /* Internal writes from the PPU are routed here. */

		uint8_t ReadCPU(uint16_t address);              /* Reads from the CPU are routed here. */
		void WriteCPU(uint16_t address, uint8_t value); /* Writes from the CPU are routed here. */

		/* ----------------------------------------------------------------------------------------------- */

		void SetMirroringMode();

		/* Read from PPU memory without causing any emulation side effects. */
		uint8_t PeekMemory(uint16_t address) { return ppu_memory[address]; };

		uint32_t* GetVideoBuffer() { return video_buffer; };

		uint64_t CycleCount() { return cycle_count; };

	private:
		void ProcessPrerenderScanline();
		void ProcessVisibleScanline();
		void ProcessPostrenderScanline();

		void ReadTile();

		void DrawPixel();
		void DrawPixel(uint16_t x, uint16_t y, uint32_t color);
		void DrawPixel(uint16_t x, uint16_t y, uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha);

	private:
		NESSystem* nes_system;

		/* Pointer to ppu_buffer that acts as the link between PPU and RF circuit on NES. */
		uint32_t* video_buffer;
		/* Internal PPU buffer that holds screen data. */
		uint32_t ppu_buffer[NES_SCREEN_WIDTH * NES_SCREEN_HEIGHT];

		/* The NES PPU can address up to 16kB (0x4000 bytes) of memory.
		   However only 2kB is stored directly on the PPU, with the rest depending on cartridge mapping.
		   It is emulated as full 16kB here for now.*/
		uint8_t ppu_memory[0x1000];

		/* BACKGROUND RENDERING----------------------------------------------------------------- */

		uint16_t pattern_table_shift_register_1;
		uint16_t pattern_table_shift_register_2;

		uint8_t pattern_attribute_shift_register_1;
		uint8_t pattern_attribute_shift_register_2;

		/* SPRITE RENDERING -------------------------------------------------------------------- */

		/* The object attribute memory table or OAM is a collection of 64 entries 4 bytes wide of sprites that the PPU renders. */
		uint8_t object_attribute_memory[256];
		/* This secondary OAM is used internally by the PPU to hold 8 sprites being rendered that scanline. */
		uint8_t second_attribute_memory[32];

		uint8_t pattern_table_shift_register_oam_1[8];
		uint8_t pattern_table_shift_register_oam_2[8];

		uint8_t attribute_latch[8];

		uint8_t x_position_counter[8];

		/* PPU REGISTERS ----------------------------------------------------------------------- */

		uint8_t ppu_ctrl;     /* General PPU control register. Controls NMI, Master/Slave, Sprite Height, Background Select, Sprite Tile Select, Increment Mode and Nametable Select. */
		uint8_t ppu_mask;     /* Rendering PPU control register. Controls Color Emphasis, Sprite Priority, Background Enable, Sprite Left Column Enable, Background Left Column Enable and Greyscale. */
		uint8_t ppu_status;   /* General PPU status register. Normally read only. Informs of VBlank Start, Sprite 0 Hit, and the buggy Sprite Overflow flag. */
		uint8_t ppu_scroll_x; /* Latch/Register controlling PPU scrolling. Takes 2 successive writes. Write 1 is X value, write 2 is Y value (X value stored here). */
		uint8_t ppu_scroll_y; /* Latch/Register controlling PPU scrolling. Takes 2 successive writes. Write 1 is X value, write 2 is Y value (Y value stored here). */
		uint16_t ppu_address; /* Latch/Register containing PPU read/write address. Takes 2 successive writes. Write 1 is MSB, write 2 is LSB. */
		uint8_t oam_address;  /* Latch/Register containing OAM read/write address. Takes 1 write. */

		uint8_t ppu_scroll_write_counter;  /* Internal counter keeping track of writes to PPUSCROLL. */
		uint8_t ppu_address_write_counter; /* Internal counter keeping track of writes to PPUADDR. */

		uint16_t current_scanline; /* Internal counter keeping track of the current scanline. */
		uint16_t current_cycle;    /* Internal counter keeping track of the current cycle inside the scanline. */

		uint64_t frame_count;
		uint64_t cycle_count;
};

#endif /* __PPU_HPP__ */