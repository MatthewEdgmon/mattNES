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

		/* Get a pointer to PPU buffer, needed for SDL. */
		uint32_t* GetVideoBuffer() { return ppu_buffer.data(); };

		uint16_t GetCurrentCycle() { return current_cycle; };
		uint16_t GetCurrentScanline() { return current_scanline; };

		uint64_t CycleCount() { return cycle_count; };

		static const uint16_t ScreenWidth { 256 };
		static const uint16_t ScreenHeight { 240 };

	private:
		void ProcessPrerenderScanline();
		void ProcessVisibleScanline();
		void ProcessPostrenderScanline();

		void ReadTile();

		void DrawPixel();
		void DrawPixel(int x, int y, uint32_t color);
		void DrawPixel(int x, int y, uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha);

		/* Returns true if given X and Y coordinate is within the visible section of the buffer. */
		inline bool InVisibleSection(int x, int y) {
			if(-1 < x && x <= ScreenWidth) {
				if(-1 < y && y <= ScreenHeight) {
					return true;
				}
			}
			return false;
		}

	private:
		NESSystem* nes_system;

		/* Internal PPU buffer that holds screen data. */
		std::vector<uint32_t> ppu_buffer { 0 };

		/* The NES PPU can address up to 16kB (0x4000 bytes) of memory.
		   However only 2kB is stored directly on the PPU, with the rest depending on cartridge mapping.
		   It is emulated as full 16kB here for now.*/
		std::vector<uint8_t> ppu_memory { 0 };

		/* BACKGROUND RENDERING----------------------------------------------------------------- */

		uint16_t pattern_table_shift_register_1 { 0 };
		uint16_t pattern_table_shift_register_2 { 0 };

		uint8_t pattern_attribute_shift_register_1 { 0 };
		uint8_t pattern_attribute_shift_register_2 { 0 };

		/* SPRITE RENDERING -------------------------------------------------------------------- */

		/* The object attribute memory table or OAM is a collection of 64 entries 4 bytes wide (256 bytes total) of sprites that the PPU renders. */
		std::vector<uint8_t> object_attribute_memory { 0 };
		/* This secondary OAM is used internally by the PPU to hold 8 sprites being rendered that scanline. */
		std::vector<uint8_t> second_attribute_memory { 0 };

		uint8_t pattern_table_shift_register_oam_1[8] { 0 };
		uint8_t pattern_table_shift_register_oam_2[8] { 0 };

		uint8_t attribute_latch[8]  { 0 };

		uint8_t x_position_counter[8] { 0 };

		/* PPU REGISTERS ----------------------------------------------------------------------- */

		uint8_t ppu_ctrl { 0 };     /* General PPU control register. Controls NMI, Master/Slave, Sprite Height, Background Select, Sprite Tile Select, Increment Mode and Nametable Select. */
		uint8_t ppu_mask { 0 };     /* Rendering PPU control register. Controls Color Emphasis, Sprite Priority, Background Enable, Sprite Left Column Enable, Background Left Column Enable and Greyscale. */
		uint8_t ppu_status { 0 };   /* General PPU status register. Normally read only. Informs of VBlank Start, Sprite 0 Hit, and the buggy Sprite Overflow flag. */
		uint8_t ppu_scroll_x { 0 }; /* Latch/Register controlling PPU scrolling. Takes 2 successive writes. Write 1 is X value, write 2 is Y value (X value stored here). */
		uint8_t ppu_scroll_y { 0 }; /* Latch/Register controlling PPU scrolling. Takes 2 successive writes. Write 1 is X value, write 2 is Y value (Y value stored here). */
		uint16_t ppu_address { 0 }; /* Latch/Register containing PPU read/write address. Takes 2 successive writes. Write 1 is MSB, write 2 is LSB. */
		uint8_t oam_address { 0 };  /* Latch/Register containing OAM read/write address. Takes 1 write. */

		uint8_t ppu_scroll_write_counter { 0 };  /* Internal counter keeping track of writes to PPUSCROLL. */
		uint8_t ppu_address_write_counter { 0 }; /* Internal counter keeping track of writes to PPUADDR. */

		uint16_t current_scanline { 0 }; /* Internal counter keeping track of the current scanline. */
		uint16_t current_cycle { 0 };    /* Internal counter keeping track of the current cycle inside the scanline. */

		uint64_t frame_count { 0 };
		uint64_t cycle_count { 0 };

		uint32_t palette[64] = {
			0x7C7C7C, 0x0000FC, 0x0000BC, 0x4428BC, 0x940084, 0xA80020, 0xA81000, 0x881400,
			0x503000, 0x007800, 0x006800, 0x005800, 0x004058, 0x000000, 0x000000, 0x000000,
			0xBCBCBC, 0x0078F8, 0x0058F8, 0x6844FC, 0xD800CC, 0xE40058, 0xF83800, 0xE45C10,
			0xAC7C00, 0x00B800, 0x00A800, 0x00A844, 0x008888, 0x000000, 0x000000, 0x000000,
			0xF8F8F8, 0x3CBCFC, 0x6888FC, 0x9878F8, 0xF878F8, 0xF85898, 0xF87858, 0xFCA044,
			0xF8B800, 0xB8F818, 0x58D854, 0x58F898, 0x00E8D8, 0x787878, 0x000000, 0x000000,
			0xFCFCFC, 0xA4E4FC, 0xB8B8F8, 0xD8B8F8, 0xF8B8F8, 0xF8A4C0, 0xF0D0B0, 0xFCE0A8,
			0xF8D878, 0xD8F878, 0xB8F8B8, 0xB8F8D8, 0x00FCFC, 0xF8D8F8, 0x000000, 0x000000
		};
};

#endif /* __PPU_HPP__ */