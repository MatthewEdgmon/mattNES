/**
 * Copyright (C) 2023 by Matthew Edgmon
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
#include <sstream>
#include <vector>

#include "../BitOps.hpp"
#include "../HexOutput.hpp"

#include "NESSystem.hpp"
#include "Cartridge.hpp"
#include "APU.hpp"
#include "CPU.hpp"
#include "PPU.hpp"

uint16_t CalculateIndirect(CPU* cpu, uint8_t operand1, uint8_t operand2) {
	
	uint16_t target = 0x0000;

	/* Bug on NMOS 6502: JMP ignores page crossing on page barrier (0xXXFF). */
	if(operand1 == 0xFF) {
		/* Ignore LSB, address fetched is from low byte 0x10FF and high byte 0x1000. */
		target  = cpu->PeekMemory((operand2 << 8) + operand1);
		target += cpu->PeekMemory((operand2 << 8)) << 8;
	} else {
		/* Perform expected behaviour. */
		target  = cpu->PeekMemory((operand2 << 8) + operand1);
		target += cpu->PeekMemory((operand2 << 8) + operand1 + 1) << 8;
	}

	return target;
}

uint16_t CalculateIndirectIndexed(CPU* cpu, uint8_t operand1) {
	/* Read address stored at operand1 in zero page. */
	return ((cpu->PeekMemory(operand1 + 1) << 8) + cpu->PeekMemory(operand1) + cpu->GetRegisterY());
}

std::string CPU::StepDisassembler() {

	/*
	 * When disassembling, keep in mind the change the instruction is doing to the processor state has not happened yet.
	 */

	std::ostringstream buffer;

	dis_instruction = PeekMemory(program_counter);
	dis_operand1    = PeekMemory(program_counter + 1);
	dis_operand2    = PeekMemory(program_counter + 2);

	buffer << HEX4X(program_counter) << "  ";
	
	/* Output 1 - 3 bytes depending on instruction sizing. */
	switch(instruction_sizes[dis_instruction]) {
		case 1:
			buffer << HEX2X(dis_instruction) << "       ";
			break;
		case 2:
			buffer << HEX2X(dis_instruction) << " " << HEX2X(dis_operand1) << "    ";
			break;
		case 3:
			buffer << HEX2X(dis_instruction) << " " << HEX2X(dis_operand1) << " " << HEX2X(dis_operand2) << " ";
			break;
		default:
			buffer << HEX2X(dis_instruction) << "       ";
			break;
	}

	/* Output * before instruction name if illegal opcode. */
	if(instruction_is_illegal[dis_instruction]) {
		buffer << "*";
	} else {
		buffer << " ";
	}

	/* Output the instruction name. */
	buffer << instruction_names[dis_instruction];

	/* Output the instruction arguments, formatted based on the addressing mode. */
	switch(instruction_mode[dis_instruction]) {
		case IMP:
			break;
		case ACU:
			buffer << " A";
			break;
		case IMM:
			buffer << " #$" << HEX2X(dis_operand1);
			break;
		case ZPG:
			buffer << " $" << HEX2X(dis_operand1) << " = " << HEX2X(PeekMemory(dis_operand1));
			break;
		case ZPX:
			buffer << " $" << HEX2X(dis_operand1) << ",X @ " << HEX2X(dis_operand1 + register_x) << " = " << HEX2X(PeekMemory(static_cast<uint8_t>(dis_operand1 + register_x)));
			break;
		case ZPY:
			buffer << " $" << HEX2X(dis_operand1) << ",Y @ " << HEX2X(dis_operand1 + register_y) << " = " << HEX2X(PeekMemory(static_cast<uint8_t>(dis_operand1 + register_y)));
			break;
		case REL:
			// Always off by two for some reason??
			buffer << " $" << HEX4X(program_counter + (int8_t)dis_operand1 + 2);
			break;
		case ABS:
			buffer << " $" << HEX4X((dis_operand2 << 8) + dis_operand1);
			if(instruction_opcode[dis_instruction] != JMP && instruction_opcode[dis_instruction] != JSR) {
				buffer << " = " << HEX2X(PeekMemory((dis_operand2 << 8) + dis_operand1));
			}
			break;
		case ABX:
			buffer << " $" << HEX4X((dis_operand2 << 8) + dis_operand1) << ",X @ " << HEX4X((dis_operand2 << 8) + dis_operand1 + register_x) << " = " << HEX2X(PeekMemory((dis_operand2 << 8) + dis_operand1 + register_x));
			break;
		case ABY:
			buffer << " $" << HEX4X((dis_operand2 << 8) + dis_operand1) << ",Y @ " << HEX4X((dis_operand2 << 8) + dis_operand1 + register_y) << " = " << HEX2X(PeekMemory((dis_operand2 << 8) + dis_operand1 + register_y));
			break;
		case IND:
			buffer << " ($" << HEX4X((dis_operand2 << 8) + dis_operand1) << ") = " << HEX4X((PeekMemory((dis_operand2 << 8) + dis_operand1 + 1) << 8) + (PeekMemory((dis_operand2 << 8) + operand1 + 1)));
			break;
		case IIN:
			buffer << " ($" << HEX2X(dis_operand1) << ",X) @ " << HEX2X(dis_operand1 + register_x) << " = ";
			dis_operand1 = PeekMemory(program_counter + 1) + register_x;
			dis_operand2 = PeekMemory(program_counter + 1) + register_x + 1;
			buffer << HEX4X((PeekMemory(dis_operand2) << 8) + PeekMemory(dis_operand1)) << " = " << HEX2X(PeekMemory((PeekMemory(dis_operand2) << 8) + PeekMemory(dis_operand1)));
			break;
		case INI:
			buffer << " ($" << HEX2X(dis_operand1) << "),Y = ";
			dis_operand1 = PeekMemory(program_counter + 1);
			dis_operand2 = PeekMemory(dis_operand1 + 1);
			dis_operand1 = PeekMemory(dis_operand1);
			buffer << HEX4X((dis_operand2 << 8) + dis_operand1) << " @ " << HEX4X((dis_operand2 << 8) + dis_operand1 + register_y) << " = " << HEX2X(PeekMemory((dis_operand2 << 8) + dis_operand1 + register_y));
			break;
		default:
			buffer << " Unknown addressing mode?";
			break;
	}

	/* Pad output to column 49. */
	buffer << std::setfill(' ') << std::setw(50 - buffer.tellp());
	
	/* Ouput CPU registers. */
	buffer << " A:" << HEX2X(register_a);
	buffer << " X:" << HEX2X(register_x);
	buffer << " Y:" << HEX2X(register_y);
	buffer << " P:" << HEX2X(register_p);
	buffer << " SP:" << HEX2X(register_s);
	
	/* Output PPU status. */
	//if(nes_system->GetPPU()->GetCurrentCycle() < 10) {
	//	buffer << " CYC:  " << nes_system->GetPPU()->GetCurrentCycle();
	//} else if (nes_system->GetPPU()->GetCurrentCycle() < 100) {
	//	buffer << " CYC: " << nes_system->GetPPU()->GetCurrentCycle();
	//} else {
	//	buffer << " CYC:" << nes_system->GetPPU()->GetCurrentCycle();
	//}

	//buffer << " SL:" << nes_system->GetPPU()->GetCurrentScanline();

	buffer << '\n';

	return buffer.str();
}