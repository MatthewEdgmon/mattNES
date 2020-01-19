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

/* PPU reading/writing functions located here to ease readability. */

#include <iostream>

#include "../BitOps.hpp"
#include "../HexOutput.hpp"
#include "NESSystem.hpp"
#include "Cartridge.hpp"

#include "PPU.hpp"

uint8_t PPU::ReadPPU(uint16_t address) {

	uint8_t value = nes_system->GetFloatingBus();

         if(address >= 0x0000 && address <= 0x1FFF) { value = nes_system->GetCartridge()->GetMapper()->ReadPPU(address); } /* Normally mapped to CHR-ROM or CHR-RAM. Often bankswitched. */
    else if(address >= 0x2000 && address <= 0x2FFF) { value = ppu_memory[address]; }                                       /* Normally mapped to 2kB PPU RAM, but can be partly or fulled remapped to cartridge. */
    else if(address >= 0x3000 && address <= 0x3EFF) { value = ppu_memory[address]; }                                       /* "Usually" a mirror of 0x2000 to 0x2FFF. */
    else if(address >= 0x3F00 && address <= 0x3FFF) { value = ppu_memory[address]; }                                       /* Always mapped to PPU internal pallete control. */
	else {
		value = nes_system->GetFloatingBus();
	}

	nes_system->SetFloatingBus(value);
	return value;
}

void PPU::WritePPU(uint16_t address, uint8_t value) {

	nes_system->SetFloatingBus(value);

         if(address >= 0x0000 && address <= 0x1FFF) { nes_system->GetCartridge()->GetMapper()->WritePPU(address, value); } /* Normally mapped to CHR-ROM or CHR-RAM. Often bankswitched. */
    else if(address >= 0x2000 && address <= 0x2FFF) { ppu_memory[address] = value; }                                       /* Normally mapped to 2kB PPU RAM, but can be partly or fulled remapped to cartridge. */
    else if(address >= 0x3000 && address <= 0x3EFF) { ppu_memory[address] = value; }                                       /* "Usually" a mirror of 0x2000 to 0x2FFF. */
    else if(address >= 0x3F00 && address <= 0x3FFF) { ppu_memory[address] = value; }                                       /* Always mapped to PPU internal pallete control. */

	return;
}

uint8_t PPU::ReadCPU(uint16_t address) {

	uint8_t value = 0x00;

	/* PPUCTRL */
	if(address == 0x2000) {
		value = nes_system->GetFloatingBus();
	}

	/* PPUMASK */
	if(address == 0x2001) {
		value = nes_system->GetFloatingBus();
	}

	/* PPUSTATUS */
	if(address == 0x2002) {
		// TODO: Fill lower 5 bits with data from last register write.
		value = ppu_status;
		ppu_status = ppu_status ^ 0x80;

		/* Reading from PPUSTATUS clears both PPUSCROLL and PPUADDR latches. */
		ppu_scroll_x = 0;
		ppu_scroll_y = 0;
		ppu_address  = 0;
		ppu_address_write_counter = 0;
		ppu_scroll_write_counter  = 0;
	}

	/* OAMADDR */
	if(address == 0x2003) {
		value = nes_system->GetFloatingBus();
	}

	/* OAMDATA */
	if(address == 0x2004) {
		// TODO: Emulate this register being unreadable on older PPU revisions.
		// TODO: Reads during vertical or forced blanking do not increment OAMADDR.
		value = object_attribute_memory[oam_address];
	}

	/* PPUSCROLL */
	if(address == 0x2005) {
		value = nes_system->GetFloatingBus();
	}

	/* PPUADDR */
	if(address == 0x2006) { 
		value = nes_system->GetFloatingBus();
	}

	/* PPUDATA */
	if(address == 0x2007) {
	
		value = ppu_memory[ppu_address];

		/* Check if bit 2 is set to increment VRAM address. */
		if(ppu_ctrl & 0x02) {
			ppu_address++;
		}
	}

	nes_system->SetFloatingBus(value);
	return value;
}

void PPU::WriteCPU(uint16_t address, uint8_t value) {

	nes_system->SetFloatingBus(value);

	/* PPUCTRL */
	if(address == 0x2000) {
		ppu_ctrl = value;
	}

	/* PPUMASK */
	if(address == 0x2001) {
		ppu_mask = value;
	} 

	/* PPUSTATUS */
	if(address == 0x2002) {
		// Discard.
	}

	/* OAMADDR */
	if(address == 0x2003) {
		oam_address = value;
	}

	/* OAMDATA */
	if(address == 0x2004) {
		object_attribute_memory[oam_address] = value;
		oam_address++; // Writes always increment oam_address.
	}

	/* PPUSCROLL */
	if(address == 0x2005) { 
		switch(ppu_scroll_write_counter) {
			case 0x00: // First write, X
				ppu_address = (ppu_address + (value << 8));
				ppu_address_write_counter++;
				break;
			case 0x01: // Second write, Y, reset counter.
				ppu_address = (ppu_address + (value << 8));
				ppu_address_write_counter = 0;
				break;
			default: // wut
				ppu_address_write_counter = 0;
				return;
		}
	}

	/* PPUADDR */
	if(address == 0x2006) {

		switch(ppu_address_write_counter) {
			case 0x00: // First write, MSB
				ppu_address = (value << 8);
				ppu_address_write_counter++;
				break;
			case 0x01: // Second write, LSB, reset counter.
				ppu_address += value;
				ppu_address_write_counter = 0;
				break;
			default: // wut
				ppu_address_write_counter = 0;
				return;
		}
	}

	/* PPUDATA */
	if(address == 0x2007) {
	
		ppu_memory[ppu_address] = value;

		/* Check if bit 2 is set to increment VRAM address. */
		if(ppu_ctrl & 0x02) {
			ppu_address++;
		}
	}

	return;
}