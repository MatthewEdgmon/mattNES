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

#include <iostream>

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

	/* Clear buffer, and set up pointer to it. */
	for(size_t i = 0; i < (NES_SCREEN_WIDTH * NES_SCREEN_HEIGHT); i++) {
		ppu_buffer[i] = 0;
	}

	video_buffer = ppu_buffer;

	/* Clear VRAM */
	for(size_t i = 0; i < 0x1000; i++) {
		ppu_memory[i] = 0;
	}

	/* Clear OAM */
	for(size_t i = 0; i < 256; i++) {
		object_attribute_memory[i] = 0;
	}

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

	video_buffer = NULL;
}

void PPU::Reset(bool hard) {
	current_cycle = 0;
	current_scanline = 0;
}

void PPU::Step() {
	
	/* Visible scanlines (0 - 240). */
	if(current_scanline <= 240) {

		/* Fetch name table entry from 0x2000 */
		uint8_t nametable = ReadPPU(0x2000);

		/* Fetch corresponding attribute table entry from 0x23C0, and increment VRAM address in same row. */
		uint8_t attrtable = ReadPPU(0x23C0);

		/* Fetch the palette from the four quadrants of the attribute table. */
		uint8_t topleft, topright, bottomleft, bottomright;

		topleft = attrtable & 0x03;
		topright = attrtable & 0x0C;
		bottomleft = attrtable & 0x30;
		bottomright = attrtable & 0xC0;

		/* Bit 4 is always set because we're drawing backgrounds only for now. */
		uint8_t palette_index = 0x10;

		/* Adding top left for now. */
		palette_index += topleft;

		/* Fetch the color from the palette itself at 0x3F00 through 0x3F0F */
		uint8_t palette = ReadPPU(0x3F00 + palette_index);

		/* Tiles are stored as 16 bytes, the first 8 of which are the LSBs of a pixel, and the next 8 the MSBs.*/
		uint8_t tile_lsb[8];
		uint8_t tile_msb[8];

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

		/* Get the color emphasis. */
		uint8_t emphasis = 0x00;

		emphasis  = BitCheck(ppu_mask, PPU_MASK_EMPHASIZE_RED);
		emphasis += BitCheck(ppu_mask, PPU_MASK_EMPHASIZE_GREEN) << 1;
		emphasis += BitCheck(ppu_mask, PPU_MASK_EMPHASIZE_BLUE) << 2;

		uint32_t test_color;

		/* Generate a new test color every frame just to test VBlank stuff. */
		if(frame_count % 2 == 0) {
			test_color = 0xFF00FFFF;
		} else {
			test_color = 0xFFFF00FF;
		}

		DrawPixel(current_cycle, current_scanline, test_color);

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

	if(current_cycle >= 1   && current_cycle <= 256) {
		DrawPixel();
	}

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

	uint32_t buffer_position = (current_scanline * NES_SCREEN_WIDTH) + current_cycle;

	uint8_t pallete = 0;

	//uint8_t pixel_color = pattern_table_shift_register_1;

	ppu_buffer[buffer_position];

}

void PPU::DrawPixel(uint16_t x, uint16_t y, uint32_t color) {

	// TODO: Turn these into assertations.
	if(x > NES_SCREEN_WIDTH) {
		return;
	}

	if(y > NES_SCREEN_HEIGHT) {
		return;
	}

	ppu_buffer[(y * NES_SCREEN_WIDTH) + x] = color;

	return;
}

void PPU::DrawPixel(uint16_t x, uint16_t y, uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha) {

	// TODO: Turn these into assertations.
	if(x > NES_SCREEN_WIDTH) {
		return;
	}

	if(y > NES_SCREEN_HEIGHT) {
		return;
	}

	ppu_buffer[(y * NES_SCREEN_WIDTH) + x] = (alpha << 24) + (red << 16) + (green << 8) + (blue);

	return;
}