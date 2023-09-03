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
#include <string>
#include <sstream>

#include <SDL.h>

#include "../BitOps.hpp"
#include "../HexOutput.hpp"
#include "APU.hpp"
#include "ControllerIO.hpp"
#include "Cartridge.hpp"
#include "CPU.hpp"
#include "PPU.hpp"
#include "NESSystem.hpp"

NESSystem::NESSystem(cpu_emulation_mode_t cpu_type, ppu_emulation_mode_t ppu_type, region_emulation_mode_t region) {
	cpu_emulation_mode = cpu_type;
	ppu_emulation_mode = ppu_type;
	region_emulation_mode = region;

	memory = new uint8_t[MEMORY_SIZE];
}

NESSystem::~NESSystem() {
	delete memory;
}

void NESSystem::Initialize(std::string rom_file_name) {
	floating_bus_value = 0;

	controller_io = std::make_unique<ControllerIO>(this);
	controller_io->Initialize();

	cartridge = std::make_unique<Cartridge>(this);
	cartridge->Initialize();
	cartridge->OpenFile(rom_file_name);

	apu = std::make_unique<APU>(this);
	apu->Initialize();

	ppu = std::make_unique<PPU>(this);
	ppu->Initialize();

	cpu = std::make_unique<CPU>(this);
	cpu->Initialize();

	Reset(true);
}

void NESSystem::Shutdown() {
	cpu->Shutdown();
	ppu->Shutdown();
	apu->Shutdown();
}

void NESSystem::Reset(bool hard) {
	controller_io->Reset();
	apu->Reset(hard);
	cpu->Reset(hard);
	ppu->Reset(hard);
}

void NESSystem::Frame() {

	static int pal_counter = 0;

	if(region_emulation_mode == RegionEmulationMode::NTSC) {
		/* For NTSC, there is exactly three PPU steps per CPU step. */
		cpu->Step();
		ppu->Step();
		ppu->Step();
		ppu->Step();
		apu->Step();
	} else if(region_emulation_mode == RegionEmulationMode::PAL) {
		/* TODO: Handle PAL emulation, which is 3.2 PPU steps per CPU step. */
		cpu->Step();
		ppu->Step();
		ppu->Step();
		ppu->Step();

		// Every fifth PAL cyle, give the PPU an extra step.
		if(pal_counter == 5) {
			ppu->Step();
			pal_counter = 0;
		} else {
			pal_counter++;
		}

		apu->Step();
	}
	
	cycles++;
}

uint8_t NESSystem::Read(uint16_t address) {
		
	uint8_t value = 0x00;

	     if(address >= 0x0000 && address <= 0x1FFF) { value = memory[(address & 0x7FF)]; }                /* Zero Page, mirrored every 0x800 bytes. */
	else if(address >= 0x2000 && address <= 0x3FFF) { value = ppu->ReadExternal((address & 0x2007)); }    /* PPU Register MMIO, mirrored every 8 bytes. */
	else if(address >= 0x4000 && address <= 0x4013) { value = apu->ReadExternal(address); }               /* APU. */
	else if(address == 0x4014)                      { value = floating_bus_value; }                       /* OAMDMA. */
	else if(address == 0x4015)                      { value = apu->ReadExternal(address); }               /* APU. */
	else if(address >= 0x4016 && address <= 0x4017) { value = controller_io->Read(address); }             /* I/O. */
	else if(address >= 0x4018 && address <= 0x401F) { value = apu->ReadExternal(address); }               /* APU. */
	else if(address >= 0x4020 && address <= 0xFFFF) { value = cartridge->GetMapper()->ReadCPU(address); } /* Cartridge Memory Space (will ONLY be CPU here). */
	else {
		value = floating_bus_value;
	}

	SetFloatingBus(value);
	return value;
}

void NESSystem::Write(uint16_t address, uint8_t value) {
	
	SetFloatingBus(value);

		 if(address >= 0x0000 && address <= 0x1FFF) { memory[(address & 0x7FF)] = value; return; }               /* Zero Page, mirrored every 0x800 bytes. */
	else if(address >= 0x2000 && address <= 0x3FFF) { ppu->WriteExternal((address & 0x2007), value); return; }   /* PPU Register MMIO, mirrored every 8 bytes. */
	else if(address >= 0x4000 && address <= 0x4013) { apu->WriteExternal(address, value); return; }              /* APU. */
	else if(address == 0x4014)                      { ObjectAttributeMemoryDMA(value); }                         /* OAMDMA. */
	else if(address == 0x4015)                      { apu->WriteExternal(address, value); return; }              /* APU. */
	else if(address == 0x4016)                      { controller_io->Write(address, value); return; }            /* I/O. */
	else if(address >= 0x4017 && address <= 0x401F) { apu->WriteExternal(address, value); return; }              /* APU. */
	else if(address >= 0x4020 && address <= 0xFFFF) { cartridge->GetMapper()->WriteCPU(address, value); return; } /* Cartridge Memory Space (will ONLY be CPU here). */

	return;
}

std::string NESSystem::TestInfo() {

	std::ostringstream test_output;

	/* Check to see which test ROM type is running. */
	if((Read(0x6001) == 0xDE) && (Read(0x6002) == 0xB0) && (Read(0x6003) == 0x61)) {

		if(Read(0x6000) == 0x81) {
			test_output << "Test ROM needs reset (1 sec delay). Resetting...\n";
			// TODO: Remove this SDL_Delay when test suite script is made.
			SDL_Delay(1000);
			Reset(false);
			return test_output.str();
		} else if(Read(0x6000) == 0x80) {
			test_output << "Test ROM needs reset. Resetting...\n";
			return test_output.str();
		} else {
			test_output << "Test ROM is finished. Result code is " << HEX2X(Read(0x6000)) << ". Output: ";

			uint8_t current_character = 0xFF;
			uint16_t current_address = 0x6004;

			while(current_character != 0) {
				current_character = Read(current_address);
				test_output << current_character;
				current_address++;
			}

			test_output << '\n';
		}
	} else {
		test_output << "Test ROM results: " << HEX2X(Read(0x0002)) << "h, " << HEX2X(Read(0x0003)) << "h\n";
	}

	return test_output.str();
}

void NESSystem::ObjectAttributeMemoryDMA(uint8_t value) {
	
	/* Check to see if on even or odd CPU cycle. */
	//if(cycles % 2 != 0) {
	//	cycles++;
	//}

	// 1 dummy read cycle.
	//cycles++;

	const uint16_t oam_dma_page = 0x0100 * value;

	// 256 Alternating Read/Write cycles.
	for(uint16_t i = 0; i < 256; i++) {
		ppu->WriteOAM(Read(oam_dma_page + i));
		//cycles += 2;
	}
}