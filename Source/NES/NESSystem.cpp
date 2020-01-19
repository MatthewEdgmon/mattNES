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
#include "ControllerIO.hpp"
#include "Cartridge.hpp"
#include "APU.hpp"
#include "CPU.hpp"
#include "PPU.hpp"

#include "NESSystem.hpp"

NESSystem::NESSystem() {

}

NESSystem::~NESSystem() {

}

void NESSystem::Initialize(std::string rom_file_name) {
	floating_bus_value = 0;

	controller_io = new ControllerIO(this);
	controller_io->Initialize();

	cartridge = new Cartridge(this);
	cartridge->Initialize();
	cartridge->OpenFile(rom_file_name);

	apu = new APU(this);
	apu->Initialize();

	ppu = new PPU(this);
	ppu->Initialize();

	cpu = new CPU(this);
	cpu->Initialize();

	Reset(true);
}

void NESSystem::Shutdown() {
	apu->Shutdown();
	ppu->Shutdown();
	cpu->Shutdown();

	delete cpu;
	delete ppu;
	delete apu;
	delete cartridge;
	delete controller_io;
}

void NESSystem::Reset(bool hard) {
	controller_io->Reset();
	apu->Reset(hard);
	cpu->Reset(hard);
	ppu->Reset(hard);
}

void NESSystem::Frame() {

	/* For NTSC, there is exactly three PPU steps per CPU step. */
	ppu->Step();
	ppu->Step();
	ppu->Step();
	apu->Step();
	cpu->Step();

	/* TODO: Handle PAL emulation, which is 3.2 PPU steps per CPU step. */
}

void NESSystem::DumpTestInfo() {

	if((GetCPU()->ReadDebug(0x6001) != 0xDE) || (GetCPU()->ReadDebug(0x6002) != 0xB0) || (GetCPU()->ReadDebug(0x6003) != 0x61)) {
		std::cout << std::endl << "Test ROM not detected." << std::endl;
		return;
	}

	if(GetCPU()->ReadDebug(0x6000) == 0x81) {
		std::cout << std::endl << "Test ROM needs reset." << std::endl;
		return;
	} else if(GetCPU()->ReadDebug(0x6000) == 0x80) {
		std::cout << std::endl << "Test ROM still testing." << std::endl;
		return;
	} else {
		std::cout << "[DEBUG 0x6004]: " << std::endl;
	}

	uint16_t debug_location = 0x6004;
	uint8_t debug_char = 0xFF;

	while(debug_char != 0) {
		debug_char = GetCPU()->ReadDebug(debug_location);
		std::cout << debug_char;
		debug_location++;
	}

	return;
}
