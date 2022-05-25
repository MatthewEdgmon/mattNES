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

#include <iterator>
#include <iostream>
#include <vector>

#include "../BitOps.hpp"
#include "../HexOutput.hpp"
#include "NESSystem.hpp"
#include "Cartridge.hpp"
#include "CPU.hpp"
#include "PPU.hpp"

PPU::PPU(NESSystem* nes_system) : nes_system(nes_system) {

}

PPU::~PPU() {

}

void PPU::Initialize() {

	/* Setup and clear PPU video buffer, and set up pointer to it. */
	ppu_buffer.resize(ScreenWidth * (ScreenHeight + 1) + 1, 0);

	/* Setup and clear VRAM */
	// TODO: the memory is not always actually this size, it depends on the cartridge.
	// TODO: Does the top of the memory always hold the palette values even on startup? Is it loaded by the cartridge?
	ppu_memory.resize(0x4000, 0);

	/* Setup and clear primary/secondary OAM */
	object_attribute_memory.resize(256, 0);
	second_attribute_memory.resize(256, 0);

	ppu_ctrl = 0;
	ppu_mask = 0;
	ppu_status = 0x80;
	ppu_address = 0;
	oam_address = 0;

	ppu_scroll_write_counter = 0;
	ppu_address_write_counter = 0;

	current_cycle = 0;
	current_scanline = 0;

	frame_count = 0;
	cycle_count = 0;
}

void PPU::Shutdown() {

	ppu_ctrl = 0;
	ppu_mask = 0;
	ppu_status = 0x80;
	ppu_address = 0;
	oam_address = 0;

	ppu_scroll_write_counter = 0;
	ppu_address_write_counter = 0;
}

void PPU::Reset(bool hard) {
	current_cycle = 0;
	current_scanline = 241;
}

void PPU::Step() {
	
	/* Visible scanlines (0 - 240). */
	if(current_scanline <= 240) {

		uint16_t nametable_base_address = 0x2000;

		/* Bits 0 and 1 of PPUCTRL (0x2000 write) determine base nametable address. */
		switch(ppu_ctrl & 0x03) {
			case 0:
				nametable_base_address = 0x2000;
			case 1:
				nametable_base_address = 0x2400;
			case 2:
				nametable_base_address = 0x2800;
			case 3:
				nametable_base_address = 0x2C00;
			default:
				break;
		}

		/* The attribute table base address is determed by the nametable base address. */
		uint16_t attribute_table_base_address = nametable_base_address + 0x03C0;

		/* Bit 4 of PPUCTRL (0x2000 write) determine background patterntable address. */
		uint16_t background_patterntable_address = (BitCheck(ppu_ctrl, 4) << 12);

		/* Fetch name table entry. */
		uint16_t nametable_entry = ReadPPU(nametable_base_address);

		/* Fetch attribute table entry. */
		uint16_t attribute_entry = ReadPPU(attribute_table_base_address);

		/* Bit 13 of the address into PPU memory controls whether the pattern table is "left" (0x0000 - 0x0FFF) or "right" (0x1000 - 0x1FFF). */
		if(ppu_address & 0x1000) {
			uint16_t pattern_entry = ReadPPU(0x0000 + current_cycle);
			pattern_entry |= ReadPPU(0x0001 + current_cycle);
		} else {
			uint16_t pattern_entry = ReadPPU(0x1000 + current_cycle);
			pattern_entry |= ReadPPU(0x1001 + current_cycle);
		}

		// Each of these come together to form a two-bit value that chooses from one of...

		// The palettes. Palette memory is structed as follows.
		// 0x3F00 -> 1-byte Index into NES palette for background color.
		// 0x3F01 -> 4-byte Three colors, each of which is index into NES palette. Last byte is mirror of 0x3F00. Background only.
		// 0x3F05 -> 4-byte Three colors, each of which is index into NES palette. Last byte is mirror of 0x3F00. Background only.
		// 0x3F09 -> 4-byte Three colors, each of which is index into NES palette. Last byte is mirror of 0x3F00. Background only.
		// 0x3F0D -> 4-byte Three colors, each of which is index into NES palette. Last byte is mirror of 0x3F00. Background only.
		// 0x3F11 -> 4-byte Three colors, each of which is index into NES palette. Last byte is mirror of 0x3F00. Sprites only.
		// 0x3F15 -> 4-byte Three colors, each of which is index into NES palette. Last byte is mirror of 0x3F00. Sprites only.
		// 0x3F19 -> 4-byte Three colors, each of which is index into NES palette. Last byte is mirror of 0x3F00. Sprites only.
		// 0x3F1D -> 4-byte Three colors, each of which is index into NES palette. Last byte is mirror of 0x3F00. Sprites only.

		/* SPRITES */

		/* Get the color emphasis. */
		uint8_t emphasis = 0x00;

		emphasis  = BitCheck(ppu_mask, PPU_MASK_EMPHASIZE_RED);
		emphasis += BitCheck(ppu_mask, PPU_MASK_EMPHASIZE_GREEN) << 1;
		emphasis += BitCheck(ppu_mask, PPU_MASK_EMPHASIZE_BLUE) << 2;

		ProcessVisibleScanline();
	}

	/* Post-render scanlines (240 - 260). */
	if(current_scanline >= 240 && current_scanline <= 260) {
		ProcessPostrenderScanline();
	}

	/* Pre-render scanline (261). */
	if(current_scanline == 261) {
		ProcessPrerenderScanline();
	}
	
	/* Check for new scanline. */
	if(current_cycle == 340) {
		current_cycle = 0;
		current_scanline++;
	} else {
		current_cycle++;
	}

	//std::cout << "[PPU] SCANLINE " << current_scanline << " CYCLE " << current_cycle << '\n';

	/* Increase absolute total cycles executed. */
	cycle_count++;
}

void PPU::WriteOAM(uint8_t value) {
	// TODO: Disable OAM writes during rendering 
	object_attribute_memory[oam_address++] = value;
}

void PPU::SetMirroringMode() {

}

void PPU::ProcessPrerenderScanline() {

	/* PPUSTATUS is reset on second cycle on the pre-render scanline. */
	if(current_cycle == 1) {
		BitClear(ppu_status, PPU_STATUS_SPRITE_OVERFLOW);
		BitClear(ppu_status, PPU_STATUS_SPRITE_0_HIT);
		BitClear(ppu_status, PPU_STATUS_VBLANK);
	}

	if(current_cycle == 321 || current_cycle == 329) {
		
	}

	/* Set new frame at the end of the pre-render line. */
	if(current_cycle == 340) {
		current_scanline = 0;
		frame_count++;
	}

	// TODO: Skip 1 cycle at cycle 340 for odd frames that have rendering enabled.

}

void PPU::ProcessVisibleScanline() {

	if(current_cycle == 0) {
		/* Idle Scanline. */
	}

	/* Visible portion of the screen. */
	if(current_cycle >= 1   && current_cycle <= 256) {

		/* Generate a new test color every frame just to test VBlank stuff. */
		/*if(frame_count % 2 == 0) {
			DrawPixel(current_cycle, current_scanline, 0xFF00FFFF);
		} else {
			DrawPixel(current_cycle, current_scanline, 0xFFFF00FF);
		}*/

		DrawPixel(current_cycle, current_scanline, palette[(current_cycle + frame_count) % 64]);

	}

	/* No longer visible. */
	if(current_cycle >= 257 && current_cycle <= 320) {

	}

	if(current_cycle >= 321 && current_cycle <= 336) {

	}

	if(current_cycle >= 337 && current_cycle <= 340) {
		ReadPPU(0x00);
		ReadPPU(0x00);
	}
}

void PPU::ProcessPostrenderScanline() {

	/* VBlank flag and NMI gets generated on cycle 1 of second post render scanline. */
	if(current_scanline == 241 && current_cycle == 1) {
		
		/* Set flag in PPUSTATUS that a video blanking period is occuring. */
		BitSet(ppu_status, PPU_STATUS_VBLANK);

		/* Generate NMI if flag in PPUCTRL set. */
		if(BitCheck(ppu_ctrl, PPU_CTRL_NMI_ENABLE)) {
			nes_system->GetCPU()->Interrupt(INTERRUPT_NMI);
		}
	}
}

void PPU::ReadTile() {
	uint16_t tile_address = (0x2000 + (ppu_address & 0x0FFF));
}

void PPU::DrawPixel() {

	uint32_t buffer_position = (current_scanline * ScreenWidth) + current_cycle;

	uint8_t pallete = 0;

	//uint8_t pixel_color = pattern_table_shift_register_1;

	ppu_buffer[buffer_position];

	return;
}

void PPU::DrawPixel(int x, int y, uint32_t color) {

#ifdef _DEBUG
	if(!InVisibleSection(x, y)) {
		// TODO: Replace this with a proper error function when we get one.
		std::cout << "ERROR: Tried to draw outside visible screen bounds at (" << x << ", " << y << ")!\n";
		return;
	}
#endif

	uint32_t buffer_position = (y * ScreenWidth) + x;

	ppu_buffer[buffer_position] = color;

	return;
}

void PPU::DrawPixel(int x, int y, uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha) {

#ifdef _DEBUG
	if (!InVisibleSection(x, y)) {
		// TODO: Replace this with a proper error function when we get one.
		std::cout << "ERROR: Tried to draw outside visible screen bounds at (" << x << ", " << y << ")!\n";
		return;
	}
#endif

	uint32_t buffer_position = (y * ScreenWidth) + x;

	ppu_buffer[buffer_position] = (alpha << 24) + (red << 16) + (green << 8) + (blue);

	return;
}