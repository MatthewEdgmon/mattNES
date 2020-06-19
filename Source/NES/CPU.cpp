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

#include <bitset>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <vector>

#include "../BitOps.hpp"
#include "../HexOutput.hpp"

#include "NESSystem.hpp"
#include "Cartridge.hpp"
#include "ControllerIO.hpp"
#include "APU.hpp"
#include "CPU.hpp"
#include "PPU.hpp"

CPU::CPU(NESSystem* nes_system) : nes_system(nes_system) {

}

CPU::~CPU() {

}

void CPU::Initialize() {

	/* Clear RAM */
	for (size_t i = 0; i < 0x800; i++) {
		// TODO: Figure out if we need to emulate random values being stored in CPU memory at bootup.
		cpu_memory[i] = 0x00;
	}

	halted = false;
	illegal_opcode_triggered = false;
	halt_on_illegal_opcode = false;
	increment_pc = false;
	test_mode = false;
	show_disassembly = true;

	operand1 = 0;
	operand2 = 0;
}

void CPU::Shutdown() {
	halted = true;
}

void CPU::Reset(bool hard) {

	vector_nmi  = Read(0xFFFA);
	vector_nmi += Read(0xFFFB) << 8;

	vector_rst  = Read(0xFFFC);
	vector_rst += Read(0xFFFD) << 8;

	vector_irq  = Read(0xFFFE);
	vector_irq += Read(0xFFFF) << 8;

	program_counter = vector_rst;
	increment_pc = true;

	/* Status register starts with IRQ interrupts disabled (NMI still fires). */
	register_p = 0x04;
	register_a = 0;
	register_x = 0;
	register_y = 0;
	register_s = 0xFD;

	std::cout << "Vectors:" << std::endl;
	std::cout << "  NMI: " << HEX4(vector_nmi) << std::endl;
	std::cout << "  IRQ: " << HEX4(vector_irq) << std::endl;
	std::cout << "  RST: " << HEX4(vector_rst) << std::endl;

	cycles = 0;
}

// TODO: Investigate whether each component needs it's own PeekMemory function. Maybe some carts reset settings on read? No idea.
uint8_t CPU::PeekMemory(uint16_t address) {

	uint8_t value = 0x00;

		 if(address >= 0x0000 && address <= 0x1FFF) { value = cpu_memory[(address & 0x7FF)]; }                                /* Zero Page, mirrored every 0x800 bytes. */
	//else if(address >= 0x2000 && address <= 0x3FFF) { value = nes_system->GetPPU()->ReadCPU((address & 0x2007)); }            /* PPU Register MMIO, mirrored every 8 bytes. */
	//else if(address >= 0x4000 && address <= 0x4013) { value = nes_system->GetAPU()->ReadCPU(address); }                       /* APU. */
	//else if(address == 0x4014)                      { value = nes_system->GetFloatingBus(); }                                 /* OAMDMA. */
	//else if(address == 0x4015)                      { value = nes_system->GetAPU()->ReadCPU(address); }                       /* APU. */
	//else if(address >= 0x4016 && address <= 0x4017) { value = nes_system->GetControllerIO()->ReadIO(address); }               /* I/O. */
	//else if(address >= 0x4018 && address <= 0x401F) { value = nes_system->GetAPU()->ReadCPU(address); }                       /* APU. */
	else if(address >= 0x4020 && address <= 0xFFFF) { value = nes_system->GetCartridge()->GetMapper()->ReadCPU(address); }    /* Cartridge Memory Space */
	else {
		value = nes_system->GetFloatingBus();
	}

	return value;
}

// TODO: Implementing cycle accuracy should probably start here. Count cycles for reading etc...
uint8_t CPU::Read(uint16_t address) {

	uint8_t value = 0x00;

	     if(address >= 0x0000 && address <= 0x1FFF) { value = cpu_memory[(address & 0x7FF)]; }                                /* Zero Page, mirrored every 0x800 bytes. */
	else if(address >= 0x2000 && address <= 0x3FFF) { value = nes_system->GetPPU()->ReadCPU((address & 0x2007)); }            /* PPU Register MMIO, mirrored every 8 bytes. */
	else if(address >= 0x4000 && address <= 0x4013) { value = nes_system->GetAPU()->ReadCPU(address); }                       /* APU. */
	else if(address == 0x4014)                      { value = nes_system->GetFloatingBus(); }                                 /* OAMDMA. */
	else if(address == 0x4015)                      { value = nes_system->GetAPU()->ReadCPU(address); }                       /* APU. */
	else if(address >= 0x4016 && address <= 0x4017) { value = nes_system->GetControllerIO()->ReadIO(address); }               /* I/O. */
	else if(address >= 0x4018 && address <= 0x401F) { value = nes_system->GetAPU()->ReadCPU(address); }                       /* APU. */
	else if(address >= 0x4020 && address <= 0xFFFF) { value = nes_system->GetCartridge()->GetMapper()->ReadCPU(address); }    /* Cartridge Memory Space */
	else {
		value = nes_system->GetFloatingBus();
	}

	nes_system->SetFloatingBus(value);
	return value;
}

void CPU::Write(uint16_t address, uint8_t value) {

	nes_system->SetFloatingBus(value);

	if(address >= 0x0000 && address <= 0x1FFF) { cpu_memory[(address & 0x7FF)] = value; return; }                             /* Zero Page, mirrored every 0x800 bytes. */
	if(address >= 0x2000 && address <= 0x3FFF) { nes_system->GetPPU()->WriteCPU((address & 0x2007), value); return; }         /* PPU Register MMIO, mirrored every 8 bytes. */
	if(address >= 0x4000 && address <= 0x4013) { nes_system->GetAPU()->WriteCPU(address, value); return; }                    /* APU. */
	if(address == 0x4014)                      { PerformOAMDMA(value); }                                                      /* OAMDMA. */
	if(address == 0x4015)                      { nes_system->GetAPU()->WriteCPU(address, value); return; }                    /* APU. */
	if(address == 0x4016)                      { nes_system->GetControllerIO()->WriteIO(address, value); return; }            /* I/O. */
	if(address >= 0x4017 && address <= 0x401F) { nes_system->GetAPU()->WriteCPU(address, value); return; }                    /* APU. */
	if(address >= 0x4020 && address <= 0xFFFF) { nes_system->GetCartridge()->GetMapper()->WriteCPU(address, value); return; } /* Cartridge Memory Space */

	return;
}

void CPU::Push(uint8_t value) {

	if(register_s == 0x00) {
		std::cout << std::endl << std::endl << "STACK OVERFLOW" << std::endl << std::endl;
		nes_system->DumpTestInfo();
		while (1);
	}

	cpu_memory[(0x0100 + register_s)] = value;
	register_s--;
}

uint8_t CPU::Pop() {
	
	if(register_s == 0xFF) {
		std::cout << std::endl << std::endl << "STACK UNDERFLOW" << std::endl << std::endl;
		nes_system->DumpTestInfo();
		while (1);
	}

	register_s++;
	uint8_t return_value = cpu_memory[(0x100 + register_s)];

	/* Clear the value on the stack location left behind. */
	cpu_memory[(0x0100 + register_s)] = 0;

	return return_value;
}

void CPU::Interrupt(interrupt_type_t interrupt_type) {

	if(interrupt_type == INTERRUPT_BRK) {
		/* Software interrupts add 1 to program counter. */
		program_counter++;
	}

	/* Push PC high byte, PC low byte, and processor flags in that order. */
	Push(program_counter >> 8);
	Push(program_counter);

	uint8_t flags_to_push = BitCheck(register_p, STATUS_BIT_NEGATIVE)          << 7 |
							BitCheck(register_p, STATUS_BIT_OVERFLOW)          << 6 |
							                                                 1 << 5 | /* Will always be 1. */
							                       (interrupt_type == INTERRUPT_BRK) << 4 | /* Set to one if software interrupt. */
							BitCheck(register_p, STATUS_BIT_DECIMAL)           << 3 |
							BitCheck(register_p, STATUS_BIT_INTERRUPT_DISABLE) << 2 |
							BitCheck(register_p, STATUS_BIT_ZERO)              << 1 |
							BitCheck(register_p, STATUS_BIT_CARRY);
	
	Push(flags_to_push);

	/* Set interrupt disable flag. */
	BitSet(register_p, STATUS_BIT_INTERRUPT_DISABLE);

	switch(interrupt_type) {
		case INTERRUPT_NMI:
			program_counter = vector_nmi;
			break;
		case INTERRUPT_BRK:
			program_counter = vector_irq;
		default:
			break;
	}
}

void CPU::PerformOAMDMA(uint8_t value) {
	
	/* Check to see if on even or odd CPU cycle. */
	if(cycles % 2 != 0) {
		cycles++;
	}

	// 1 dummy read cycle.
	cycles++;

	const uint16_t oam_dma_page = 0x0100 * value;

	// 256 Alternating Read/Write cycles.
	for(uint16_t i = 0; i < 256; i++) {
		nes_system->GetPPU()->WriteOAM(Read(oam_dma_page + i));
		cycles += 2;
	}
}