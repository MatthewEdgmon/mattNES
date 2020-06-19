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
#include <SDL.h>

NESSystem::NESSystem(cpu_emulation_mode_t cpu_type, ppu_emulation_mode_t ppu_type, region_emulation_mode_t region) {
	cpu_emulation_mode = cpu_type;
	ppu_emulation_mode = ppu_type;
	region_emulation_mode = region;
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

	if(region_emulation_mode == NTSC) {
		/* For NTSC, there is exactly three PPU steps per CPU step. */
		ppu->Step();
		ppu->Step();
		ppu->Step();
		apu->Step();
		cpu->Step();
	} else if(region_emulation_mode == PAL) {
		/* TODO: Handle PAL emulation, which is 3.2 PPU steps per CPU step. */
	}
	
}

void NESSystem::DumpTestInfo() {

	std::cout << "\nCPU STATE\n";
	std::cout << "Register A = " << HEX2(cpu->GetRegisterA()) << '\n';
	std::cout << "Register X = " << HEX2(cpu->GetRegisterX()) << '\n';
	std::cout << "Register Y = " << HEX2(cpu->GetRegisterY()) << '\n';
	std::cout << "Register S = " << HEX2(cpu->GetRegisterS()) << '\n';
	std::cout << "Register P = " << HEX2(cpu->GetRegisterP()) << '\n';
	std::cout << "Program Counter = " << HEX4(cpu->GetProgramCounter()) << '\n';
	std::cout << "Flags Set: ";
	if(BitCheck(cpu->GetRegisterP(), STATUS_BIT_NEGATIVE))          { std::cout << " NEGATIVE"; }  else { std::cout << "         "; }
	if(BitCheck(cpu->GetRegisterP(), STATUS_BIT_OVERFLOW))          { std::cout << " OVERFLOW"; }  else { std::cout << "         "; }
	if(BitCheck(cpu->GetRegisterP(), STATUS_BIT_S2))                { std::cout << " UNUSED2"; }   else { std::cout << "        "; }
	if(BitCheck(cpu->GetRegisterP(), STATUS_BIT_S1))                { std::cout << " UNUSED1"; }   else { std::cout << "        "; }
	if(BitCheck(cpu->GetRegisterP(), STATUS_BIT_DECIMAL))           { std::cout << " DECIMAL"; }   else { std::cout << "        "; }
	if(BitCheck(cpu->GetRegisterP(), STATUS_BIT_INTERRUPT_DISABLE)) { std::cout << " INTERRUPT"; } else { std::cout << "          "; }
	if(BitCheck(cpu->GetRegisterP(), STATUS_BIT_ZERO))              { std::cout << " ZERO"; }      else { std::cout << "     "; }
	if(BitCheck(cpu->GetRegisterP(), STATUS_BIT_CARRY))             { std::cout << " CARRY"; }     else { std::cout << "      "; }
	std::cout << '\n';
	
	/* Check to see which test ROM type is running. */
	if((GetCPU()->PeekMemory(0x6001) == 0xDE) && (GetCPU()->PeekMemory(0x6002) == 0xB0) && (GetCPU()->PeekMemory(0x6003) == 0x61)) {

		if(GetCPU()->PeekMemory(0x6000) == 0x81) {
			std::cout << "Test ROM needs reset. Resetting...\n";
			// TODO: Remove this SDL_Delay when test suite script is made.
			SDL_Delay(1000);
			Reset(false);
			return;
		} else if(GetCPU()->PeekMemory(0x6000) == 0x80) {
			std::cout << "Test ROM still testing." << '\n';
			std::cout << "[Test ROM (0x6004)]: ";
			//return;
		} else {
			std::cout << "Test ROM is finished.\n";
			std::cout << "[Test ROM (0x6004)]: ";
		}

		for(size_t i = 0; i < 64; i++) {
			printf("%c", GetCPU()->PeekMemory(0x6004 + i));
		}

		std::cout << '\n';

	} else {
		std::cout << "Test ROM results: " << HEX2X(GetCPU()->PeekMemory(0x0002)) << HEX2X(GetCPU()->PeekMemory(0x0003)) << '\n';
	}

	return;
}
