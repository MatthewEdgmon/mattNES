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
#include "APU.hpp"
#include "CPU.hpp"
#include "PPU.hpp"

inline uint16_t CalculateIndirect(CPU* cpu, uint8_t operand1, uint8_t operand2) {
	
	uint16_t target = 0x0000;

	/* Bug on NMOS 6502: JMP ignores page crossing on page barrier (0xXXFF). */
	if(operand1 == 0xFF) {
		/* Ignore LSB, address fetched is from low byte 0x10FF and high byte 0x1000. */
		target  = cpu->ReadDebug((operand2 << 8) + operand1);
		target += cpu->ReadDebug((operand2 << 8)) << 8;
	} else {
		/* Perform expected behaviour. */
		target  = cpu->ReadDebug((operand2 << 8) + operand1);
		target += cpu->ReadDebug((operand2 << 8) + operand1 + 1) << 8;
	}

	return target;
}

inline uint16_t CalculateIndirectIndexed(CPU* cpu, uint8_t operand1) {
	/* Read address stored at operand1 in zero page. */
	return ((cpu->ReadDebug(operand1 + 1) << 8) + cpu->ReadDebug(operand1) + cpu->GetRegisterY());
}

void CPU::StepDisassembler() {

	/*
	 * When disassembling, keep in mind the change the instruction is doing to the processor state has not happened yet.
	 * For example instruction C8 - INY must show register_y with one added to show the change that will happen.
	 */

	//std::cout << '\n';
	//std::cout << "A  = " << HEX2(register_a)    << "   X  = " << HEX2(register_x) << " Y  = " << HEX2(register_y) << '\n';
	//std::cout << "PC = " << HEX4(program_counter) << " SP = " << HEX2(register_s) << " P  = " << HEX2(register_p) << '\n';

	std::cout << "["     << HEX4(program_counter) << "] "     << HEX2X(instruction);

	switch(instruction) {
		case 0x00: std::cout << "      "                                         << " BRK" << '\n'; break;
		case 0x01: std::cout << " " << HEX2X(operand1) << "   "                  << " ORA ?INDX?" << '\n'; break;
		case 0x02: std::cout << "      "                                         << " Illegal Opcode " << HEX2(instruction) << '\n'; break;
		case 0x03: std::cout << "      "                                         << " Illegal Opcode " << HEX2(instruction) << '\n'; break;
		case 0x04: std::cout << "      "                                         << " Illegal Opcode " << HEX2(instruction) << '\n'; break;
		case 0x05: std::cout << " " << HEX2X(operand1) << "   "                  << " ORA 0x00" << HEX2X(operand1) << " = " << HEX2X(Read(operand1)) << '\n'; break;
		case 0x06: std::cout << " " << HEX2X(operand1) << "   "                  << " ASL 0x00" << HEX2X(operand1) << " = " << HEX2X(Read(operand1)) << '\n'; break;
		case 0x07: std::cout << "      "                                         << " Illegal Opcode " << HEX2(instruction) << '\n'; break;
		case 0x08: std::cout << "      "                                         << " PHP" << '\n'; break;
		case 0x09: std::cout << " " << HEX2X(operand1) << "   "                  << " ORA " << HEX2X(operand1) << '\n'; break;
		case 0x0A: std::cout << "      "                                         << " ASL" << '\n'; break;
		case 0x0B: std::cout << "      "                                         << " Illegal Opcode " << HEX2(instruction) << '\n'; break;
		case 0x0C: std::cout << "      "                                         << " Illegal Opcode " << HEX2(instruction) << '\n'; break;
		case 0x0D: std::cout << " " << HEX2X(operand1) << " " << HEX2X(operand2) << " ORA " << HEX4((operand2 << 8) + operand1) << " = " << HEX2(Read((operand2 << 8) + operand1)) << '\n'; break;
		case 0x0E: std::cout << " " << HEX2X(operand1) << " " << HEX2X(operand2) << " ASL " << HEX4((operand2 << 8) + operand1) << " = " << HEX2(Read((operand2 << 8) + operand1)) << '\n'; break;
		case 0x0F: std::cout << "      "                                         << " Illegal Opcode " << HEX2(instruction) << '\n'; break;
		case 0x10: std::cout << " " << HEX2X(operand1) << "   "                  << " BPL " << HEX4((program_counter + (int8_t)operand1) + 2) << '\n'; break;
		case 0x11: std::cout << " " << HEX2X(operand1) << "   "                  << " ORA INDY" << '\n'; break;
		case 0x12: std::cout << "      "                                         << " Illegal Opcode " << HEX2(instruction) << '\n'; break;
		case 0x13: std::cout << "      "                                         << " Illegal Opcode " << HEX2(instruction) << '\n'; break;
		case 0x14: std::cout << "      "                                         << " Illegal Opcode " << HEX2(instruction) << '\n'; break;
		case 0x15: std::cout << " " << HEX2X(operand1) << "   "                  << " ORA ZPGX" << '\n'; break;
		case 0x16: std::cout << " " << HEX2X(operand1) << "   "                  << " ASL ZPGX" << '\n'; break;
		case 0x17: std::cout << "      "                                         << " Illegal Opcode " << HEX2(instruction) << '\n'; break;
		case 0x18: std::cout << "      "                                         << " CLC" << '\n'; break;
		case 0x19: std::cout << " " << HEX2X(operand1) << " " << HEX2X(operand2) << " ORA" << '\n'; break;
		case 0x1A: std::cout << "      "                                         << " Illegal Opcode " << HEX2(instruction) << '\n'; break;
		case 0x1B: std::cout << "      "                                         << " Illegal Opcode " << HEX2(instruction) << '\n'; break;
		case 0x1C: std::cout << "      "                                         << " Illegal Opcode " << HEX2(instruction) << '\n'; break;
		case 0x1D: std::cout << " " << HEX2X(operand1) << " " << HEX2X(operand2) << " ORA " << HEX4((operand2 << 8) + operand1 + register_x) << '\n';
		case 0x1E: std::cout << " " << HEX2X(operand1) << " " << HEX2X(operand2) << " ASL " << HEX4((operand2 << 8) + operand1 + register_x) << '\n';
		case 0x1F: std::cout << "      "                                         << " Illegal Opcode" << '\n'; break;
		case 0x20: std::cout << " " << HEX2X(operand1) << " " << HEX2X(operand2) << " JSR " << HEX4((operand2 << 8) + operand1) << '\n'; break;
		case 0x21: std::cout << " " << HEX2X(operand1) << "   "                  << " AND" << '\n'; break;
		case 0x22: std::cout << "      "                                         << " Illegal Opcode" << '\n'; break;
		case 0x23: std::cout << "      "                                         << " Illegal Opcode" << '\n'; break;
		case 0x24: std::cout << " " << HEX2X(operand1) << "   "                  << " BIT" << '\n'; break;
		case 0x25: std::cout << " " << HEX2X(operand1) << "   "                  << " AND" << '\n'; break;
		case 0x26: std::cout << " " << HEX2X(operand1) << "   "                  << " ROL" << '\n'; break;
		case 0x27: std::cout << "      "                                         << " Illegal Opcode" << '\n'; break;
		case 0x28: std::cout << "      "                                         << " PLP" << '\n'; break;
		case 0x29: std::cout << " " << HEX2X(operand1) << "   "                  << " AND " << HEX2(operand1) << '\n'; break;
		case 0x2A: std::cout << "      "                                         << " ROL" << '\n'; break;
		case 0x2B: std::cout << "      "                                         << " Illegal Opcode " << HEX2(instruction) << '\n'; break;
		case 0x2C: std::cout << " " << HEX2X(operand1) << " " << HEX2X(operand2) << " BIT " << HEX4((operand2 << 8) + operand1) << " = " << HEX2(Read((operand2 << 8) + operand1)) << " & " << HEX2(register_a) << '\n'; break;
		case 0x2D: std::cout << " " << HEX2X(operand1) << " " << HEX2X(operand2) << " AND" << '\n'; break;
		case 0x2E: std::cout << " " << HEX2X(operand1) << " " << HEX2X(operand2) << " ROL" << '\n'; break;
		case 0x2F: std::cout << "      "                                         << " Illegal Opcode " << HEX2(instruction) << '\n'; break;
		case 0x30: std::cout << " " << HEX2X(operand1) << "   "                  << " BMI " << HEX4((program_counter + (int8_t)operand1) + 2) << '\n'; break;
		case 0x31: std::cout << " " << HEX2X(operand1) << "   "                  << " AND ?INDY?" << '\n'; break;
		case 0x32: std::cout << "      "                                         << " Illegal Opcode " << HEX2(instruction) << '\n'; break;
		case 0x33: std::cout << "      "                                         << " Illegal Opcode " << HEX2(instruction) << '\n'; break;
		case 0x34: std::cout << "      "                                         << " Illegal Opcode " << HEX2(instruction) << '\n'; break;
		case 0x35: std::cout << " " << HEX2X(operand1) << "   "                  << " AND ZPGX" << '\n'; break;
		case 0x36: std::cout << " " << HEX2X(operand1) << "   "                  << " ROL ZPGX" << '\n'; break;
		case 0x37: std::cout << "      "                                         << " Illegal Opcode " << HEX2(instruction) << '\n'; break;
		case 0x38: std::cout << "      "                                         << " SEC" << '\n'; break;
		case 0x39: std::cout << " " << HEX2X(operand1) << " " << HEX2X(operand2) << " AND " << HEX4((operand2 << 8) + operand1 + register_y) << '\n'; break;
		case 0x3A: std::cout << "      "                                         << " Illegal Opcode " << HEX2(instruction) << '\n'; break;
		case 0x3B: std::cout << "      "                                         << " Illegal Opcode " << HEX2(instruction) << '\n'; break;
		case 0x3C: std::cout << "      "                                         << " Illegal Opcode " << HEX2(instruction) << '\n'; break;
		case 0x3D: std::cout << " " << HEX2X(operand1) << " " << HEX2X(operand2) << " AND " << HEX4((operand2 << 8) + operand1 + register_x) << '\n'; break;
		case 0x3E: std::cout << " " << HEX2X(operand1) << " " << HEX2X(operand2) << " ROL " << HEX4((operand2 << 8) + operand1 + register_x) << '\n'; break;
		case 0x3F: std::cout << "      "                                         << " Illegal Opcode " << HEX2(instruction) << '\n'; break;
		case 0x40: std::cout << "      "                                         << " RTI " << HEX4((PeekStack(register_s + 2) << 8) + PeekStack(register_s + 3)) << '\n'; break;
		case 0x41: std::cout << " " << HEX2X(operand1) << "   "                  << " EOR INDX" << '\n'; break;
		case 0x42: std::cout << "      "                                         << " Illegal Opcode " << HEX2(instruction) << '\n'; break;
		case 0x43: std::cout << "      "                                         << " Illegal Opcode " << HEX2(instruction) << '\n'; break;
		case 0x44: std::cout << "      "                                         << " Illegal Opcode " << HEX2(instruction) << '\n'; break;
		case 0x45: std::cout << " " << HEX2X(operand1) << "   "                  << " EOR 0x00" << HEX2X(operand1) << " = " << HEX2X(Read(operand1)) << '\n'; break;
		case 0x46: std::cout << " " << HEX2X(operand1) << "   "                  << " LSR 0x00" << HEX2X(operand1) << " = " << HEX2X(Read(operand1)) << '\n'; break;
		case 0x47: std::cout << "      "                                         << " Illegal Opcode " << HEX2(instruction) << '\n'; break;
		case 0x48: std::cout << "      "                                         << " PHA" << '\n'; break;
		case 0x49: std::cout << " " << HEX2X(operand1) << "   "                  << " EOR " << HEX2X(operand1) << '\n'; break;
		case 0x4A: std::cout << "      "                                         << " LSR" << '\n'; break;
		case 0x4B: std::cout << "      "                                         << " Illegal Opcode " << HEX2(instruction) << '\n'; break;
		case 0x4C: std::cout << " " << HEX2X(operand1) << " " << HEX2X(operand2) << " JMP " << HEX4((operand2 << 8) + operand1) << '\n'; break;
		case 0x4D: std::cout << " " << HEX2X(operand1) << " " << HEX2X(operand2) << " EOR " << HEX4((operand2 << 8) + operand1) << " = " << HEX2(Read((operand2 << 8) + operand1)) << '\n'; break;
		case 0x4E: std::cout << " " << HEX2X(operand1) << " " << HEX2X(operand2) << " LSR " << HEX4((operand2 << 8) + operand1) << " = " << HEX2(Read((operand2 << 8) + operand1)) << '\n'; break;
		case 0x4F: std::cout << "      "                                         << " Illegal Opcode " << HEX2(instruction) << '\n'; break;
		case 0x50: std::cout << " " << HEX2X(operand1) << "   "                  << " BVC " << HEX4((program_counter + (int8_t)operand1) + 2) << '\n'; break;
		case 0x51: std::cout << " " << HEX2X(operand1) << "   "                  << " EOR ?INDY?" << '\n'; break;
		case 0x52: std::cout << "      "                                         << " Illegal Opcode " << HEX2(instruction) << '\n'; break;
		case 0x53: std::cout << "      "                                         << " Illegal Opcode " << HEX2(instruction) << '\n'; break;
		case 0x54: std::cout << "      "                                         << " Illegal Opcode " << HEX2(instruction) << '\n'; break;
		case 0x55: std::cout << " " << HEX2X(operand1) << "   "                  << " EOR ZPGX" << HEX2X(operand1) << '\n'; break;
		case 0x56: std::cout << " " << HEX2X(operand1) << "   "                  << " LSR ZPGX" << HEX2X(operand1) << '\n'; break;
		case 0x57: std::cout << "      "                                         << " Illegal Opcode " << HEX2(instruction) << '\n'; break;
		case 0x58: std::cout << "      "                                         << " CLI" << '\n'; break;
		case 0x59: std::cout << " " << HEX2X(operand1) << " " << HEX2X(operand2) << " EOR " << HEX4((operand2 << 8) + operand1 + register_y) << " = " << HEX2(Read((operand2 << 8) + operand1) + register_y) << '\n'; break;
		case 0x5A: std::cout << "      "                                         << " Illegal Opcode " << HEX2(instruction) << '\n'; break;
		case 0x5B: std::cout << "      "                                         << " Illegal Opcode " << HEX2(instruction) << '\n'; break;
		case 0x5C: std::cout << "      "                                         << " Illegal Opcode " << HEX2(instruction) << '\n'; break;
		case 0x5D: std::cout << " " << HEX2X(operand1) << " " << HEX2X(operand2) << " EOR " << HEX4((operand2 << 8) + operand1 + register_x) << " = " << HEX2(Read((operand2 << 8) + operand1) + register_x) << '\n'; break;
		case 0x5E: std::cout << " " << HEX2X(operand1) << " " << HEX2X(operand2) << " LSR " << HEX4((operand2 << 8) + operand1 + register_x) << " = " << HEX2(Read((operand2 << 8) + operand1) + register_x) << '\n'; break;
		case 0x5F: std::cout << "      "                                         << " Illegal Opcode " << HEX2(instruction) << '\n'; break;
		case 0x60: std::cout << "      "                                         << " RTS " << HEX4((PeekStack(register_s + 2) << 8) + PeekStack(register_s + 3)) << '\n'; break;
		case 0x61: std::cout << " " << HEX2X(operand1) << "   "                  << " ADC ?INDX?" << HEX2X(operand1) << '\n'; break;
		case 0x62: std::cout << "      "                                         << " Illegal Opcode " << HEX2(instruction) << '\n'; break;
		case 0x63: std::cout << "      "                                         << " Illegal Opcode " << HEX2(instruction) << '\n'; break;
		case 0x64: std::cout << "      "                                         << " Illegal Opcode " << HEX2(instruction) << '\n'; break;
		case 0x65: std::cout << " " << HEX2X(operand1) << "   "                  << " ADC 0x00" << HEX2X(operand1) << " = " << HEX2X(Read(operand1)) << '\n'; break;
		case 0x66: std::cout << " " << HEX2X(operand1) << "   "                  << " ROR 0x00" << HEX2X(operand1) << " = " << HEX2X(Read(operand1)) << '\n'; break;
		case 0x67: std::cout << "      "                                         << " Illegal Opcode " << HEX2(instruction) << '\n'; break;
		case 0x68: std::cout << "      "                                         << " PLA" << '\n'; break;
		case 0x69: std::cout << " " << HEX2X(operand1) << "   "                  << " ADC " << HEX2X(operand1) << '\n'; break;
		case 0x6A: std::cout << "      "                                         << " ROR" << '\n'; break;
		case 0x6B: std::cout << "      "                                         << " Illegal Opcode " << HEX2(instruction) << '\n'; break;
		case 0x6C: std::cout << " " << HEX2X(operand1) << " " << HEX2X(operand2) << " JMP " << HEX4((operand2 << 8) + operand1) << " = " << HEX4(CalculateIndirect(this, operand1, operand2)) << '\n'; break;
		case 0x6D: std::cout << " " << HEX2X(operand1) << " " << HEX2X(operand2) << " ADC " << HEX4((operand2 << 8) + operand1) << " = " << HEX2(Read((operand2 << 8) + operand1)) << '\n'; break;
		case 0x6E: std::cout << " " << HEX2X(operand1) << " " << HEX2X(operand2) << " ROR " << HEX4((operand2 << 8) + operand1) << " = " << HEX2(Read((operand2 << 8) + operand1)) << '\n'; break;
		case 0x6F: std::cout << "      "                                         << " Illegal Opcode " << HEX2(instruction) << '\n'; break;
		case 0x70: std::cout << " " << HEX2X(operand1) << "   "                  << " BVS " << HEX4((program_counter + (int8_t)operand1) + 2) << '\n'; break;
		case 0x71: std::cout << " " << HEX2X(operand1) << "   "                  << " ADC ?INDY?" << '\n'; break;
		case 0x72: std::cout << "      "                                         << " Illegal Opcode " << HEX2(instruction) << '\n'; break;
		case 0x73: std::cout << "      "                                         << " Illegal Opcode " << HEX2(instruction) << '\n'; break;
		case 0x74: std::cout << "      "                                         << " Illegal Opcode " << HEX2(instruction) << '\n'; break;
		case 0x75: std::cout << " " << HEX2X(operand1) << "   "                  << " ADC ZPGX" << '\n'; break;
		case 0x76: std::cout << " " << HEX2X(operand1) << "   "                  << " ROR ZPGX" << '\n'; break;
		case 0x77: std::cout << "      "                                         << " Illegal Opcode " << HEX2(instruction) << '\n'; break;
		case 0x78: std::cout << "      "                                         << " SEI" << '\n'; break;
		case 0x79: std::cout << " " << HEX2X(operand1) << " " << HEX2X(operand2) << " ADC " << HEX4((operand2 << 8) + operand1 + register_y) << " = " << HEX2(Read((operand2 << 8) + operand1) + register_y) << '\n'; break;
		case 0x7A: std::cout << "      "                                         << " Illegal Opcode " << HEX2(instruction) << '\n'; break;
		case 0x7B: std::cout << "      "                                         << " Illegal Opcode " << HEX2(instruction) << '\n'; break;
		case 0x7C: std::cout << "      "                                         << " Illegal Opcode " << HEX2(instruction) << '\n'; break;
		case 0x7D: std::cout << " " << HEX2X(operand1) << " " << HEX2X(operand2) << " ADC " << HEX4((operand2 << 8) + operand1 + register_x) << " = " << HEX2(Read((operand2 << 8) + operand1) + register_x) << '\n'; break;
		case 0x7E: std::cout << " " << HEX2X(operand1) << " " << HEX2X(operand2) << " ROR " << HEX4((operand2 << 8) + operand1 + register_x) << " = " << HEX2(Read((operand2 << 8) + operand1) + register_x) << '\n'; break;
		case 0x7F: std::cout << "      "                                         << " Illegal Opcode " << HEX2(instruction) << '\n'; break;
		case 0x80: std::cout << "      "                                         << " Illegal Opcode " << HEX2(instruction) << '\n'; break;
		case 0x81: std::cout << " " << HEX2X(operand1) << "   "                  << " STA ?INDX?" << '\n'; break;
		case 0x82: std::cout << "      "                                         << " Illegal Opcode " << HEX2(instruction) << '\n'; break;
		case 0x83: std::cout << "      "                                         << " Illegal Opcode " << HEX2(instruction) << '\n'; break;
		case 0x84: std::cout << " " << HEX2X(operand1) << "   "                  << " STY 0x00" << HEX2X(operand1) << " = " << HEX2X(register_y) << '\n'; break;
		case 0x85: std::cout << " " << HEX2X(operand1) << "   "                  << " STA 0x00" << HEX2X(operand1) << " = " << HEX2X(register_a) << '\n'; break;
		case 0x86: std::cout << " " << HEX2X(operand1) << "   "                  << " STX 0x00" << HEX2X(operand1) << " = " << HEX2X(register_x) << '\n'; break;
		case 0x87: std::cout << "      "                                         << " Illegal Opcode " << HEX2(instruction) << '\n'; break;
		case 0x88: std::cout << "      "                                         << " DEY Y = " << HEX2X((uint8_t) register_y - 1) << '\n'; break;
		case 0x89: std::cout << "      "                                         << " Illegal Opcode " << HEX2(instruction) << '\n'; break;
		case 0x8A: std::cout << "      "                                         << " TXA" << '\n'; break;
		case 0x8B: std::cout << "      "                                         << " Illegal Opcode " << HEX2(instruction) << '\n'; break;
		case 0x8C: std::cout << " " << HEX2X(operand1) << " " << HEX2X(operand2) << " STY " << HEX4((operand2 << 8) + operand1) << " = " << HEX2X(register_y) << '\n'; break;
		case 0x8D: std::cout << " " << HEX2X(operand1) << " " << HEX2X(operand2) << " STA " << HEX4((operand2 << 8) + operand1) << " = " << HEX2X(register_a) << '\n'; break;
		case 0x8E: std::cout << " " << HEX2X(operand1) << " " << HEX2X(operand2) << " STX " << HEX4((operand2 << 8) + operand1) << " = " << HEX2X(register_x) << '\n'; break;
		case 0x8F: std::cout << "      "                                         << " Illegal Opcode " << HEX2(instruction) << '\n'; break;
		case 0x90: std::cout << " " << HEX2X(operand1) << "   "                  << " BCC " << HEX4((program_counter + (int8_t)operand1) + 2) << '\n'; break;
		case 0x91: std::cout << " " << HEX2X(operand1) << "   "                  << " STA (" << HEX2(operand1) << ", Y) " << HEX4(CalculateIndirectIndexed(this, operand1)) << " = " << HEX2(register_a) << '\n'; break;
		case 0x92: std::cout << "      "                                         << " Illegal Opcode " << HEX2(instruction) << '\n'; break;
		case 0x93: std::cout << "      "                                         << " Illegal Opcode " << HEX2(instruction) << '\n'; break;
		case 0x94: std::cout << " " << HEX2X(operand1) << "   "                  << " STY 0x00" << HEX2X((uint8_t)(operand1 + register_x)) << " = " << HEX2X(register_y) << '\n'; break;
		case 0x95: std::cout << " " << HEX2X(operand1) << "   "                  << " STA 0x00" << HEX2X((uint8_t)(operand1 + register_x)) << " = " << HEX2X(register_a) << '\n'; break;
		case 0x96: std::cout << " " << HEX2X(operand1) << "   "                  << " STX 0x00" << HEX2X((uint8_t)(operand1 + register_y)) << " = " << HEX2X(register_x) << '\n'; break;
		case 0x97: std::cout << "      "                                         << " Illegal Opcode " << HEX2(instruction) << '\n'; break;
		case 0x98: std::cout << "      "                                         << " TYA" << '\n'; break;
		case 0x99: std::cout << " " << HEX2X(operand1) << " " << HEX2X(operand2) << " STA " << HEX4((operand2 << 8) + operand1 + register_y) << " = " << HEX2X(register_a) << '\n'; break;
		case 0x9A: std::cout << "      "                                         << " TXS" << '\n'; break;
		case 0x9B: std::cout << "      "                                         << " Illegal Opcode " << HEX2(instruction) << '\n'; break;
		case 0x9C: std::cout << "      "                                         << " Illegal Opcode " << HEX2(instruction) << '\n'; break;
		case 0x9D: std::cout << " " << HEX2X(operand1) << " " << HEX2X(operand2) << " STA " << HEX4((operand2 << 8) + operand1 + register_x) << " = " << HEX2X(register_a) << '\n'; break;
		case 0x9E: std::cout << "      "                                         << " Illegal Opcode " << HEX2(instruction) << '\n'; break;
		case 0x9F: std::cout << "      "                                         << " Illegal Opcode " << HEX2(instruction) << '\n'; break;
		case 0xA0: std::cout << " " << HEX2X(operand1) << "   "                  << " LDY " << HEX2(operand1) << '\n'; break;
		case 0xA1: std::cout << " " << HEX2X(operand1) << "   "                  << " LDA ?INDX? " << HEX2(operand1) << '\n'; break;
		case 0xA2: std::cout << " " << HEX2X(operand1) << "   "                  << " LDX " << HEX2(operand1) << '\n'; break;
		case 0xA3: std::cout << "      "                                         << " Illegal Opcode " << HEX2(instruction) << '\n'; break;
		case 0xA4: std::cout << " " << HEX2X(operand1) << "   "                  << " LDY 0x00" << HEX2X(operand1) << " = " << HEX2X(register_y) << '\n'; break;
		case 0xA5: std::cout << " " << HEX2X(operand1) << "   "                  << " LDA 0x00" << HEX2X(operand1) << " = " << HEX2X(register_a) << '\n'; break;
		case 0xA6: std::cout << " " << HEX2X(operand1) << "   "                  << " LDX 0x00" << HEX2X(operand1) << " = " << HEX2X(register_x) << '\n'; break;
		case 0xA7: std::cout << "      "                                         << " Illegal Opcode " << HEX2(instruction) << '\n'; break;
		case 0xA8: std::cout << "      "                                         << " TAY" << '\n'; break;
		case 0xA9: std::cout << " " << HEX2X(operand1) << "   "                  << " LDA " << HEX2(operand1) << '\n'; break;
		case 0xAA: std::cout << "      "                                         << " TAX" << '\n'; break;
		case 0xAB: std::cout << "      "                                         << " Illegal Opcode " << HEX2(instruction) << '\n'; break;
		case 0xAC: std::cout << " " << HEX2X(operand1) << " " << HEX2X(operand2) << " LDY " << HEX4((operand2 << 8) + operand1) << " = " << HEX2X(Read((operand2 << 8) + operand1)) << '\n'; break;
		case 0xAD: std::cout << " " << HEX2X(operand1) << " " << HEX2X(operand2) << " LDA " << HEX4((operand2 << 8) + operand1) << " = " << HEX2X(Read((operand2 << 8) + operand1)) << '\n'; break;
		case 0xAE: std::cout << " " << HEX2X(operand1) << " " << HEX2X(operand2) << " LDX " << HEX4((operand2 << 8) + operand1) << " = " << HEX2X(Read((operand2 << 8) + operand1)) << '\n'; break;
		case 0xAF: std::cout << "      "                                         << " Illegal Opcode " << HEX2(instruction) << '\n'; break;
		case 0xB0: std::cout << " " << HEX2X(operand1) << "   "                  << " BCS " << HEX4((program_counter + (int8_t)operand1) + 2) << '\n'; break;
		case 0xB1: std::cout << " " << HEX2X(operand1) << "   "                  << " LDA " << HEX2((Read(operand2 << 8) + operand1) + register_y) << " (from " << HEX4((Read(operand2) << 8) + Read(operand1) + register_y) << ")" << '\n'; break;
		case 0xB2: std::cout << "      "                                         << " Illegal Opcode " << HEX2(instruction) << '\n'; break;
		case 0xB3: std::cout << "      "                                         << " Illegal Opcode " << HEX2(instruction) << '\n'; break;
		case 0xB4: std::cout << " " << HEX2X(operand1) << "   "                  << " LDY 0x00" << HEX2X((uint8_t)(operand1 + register_x)) << " = " << HEX2X(Read((uint8_t)(operand1 + register_x))) << '\n'; break;
		case 0xB5: std::cout << " " << HEX2X(operand1) << "   "                  << " LDA 0x00" << HEX2X((uint8_t)(operand1 + register_x)) << " = " << HEX2X(Read((uint8_t)(operand1 + register_x))) << '\n'; break;
		case 0xB6: std::cout << " " << HEX2X(operand1) << "   "                  << " LDA 0x00" << HEX2X((uint8_t)(operand1 + register_y)) << " = " << HEX2X(Read((uint8_t)(operand1 + register_y))) << '\n'; break;
		case 0xB7: std::cout << "      "                                         << " Illegal Opcode " << HEX2(instruction) << '\n'; break;
		case 0xB8: std::cout << "      "                                         << " CLV" << '\n'; break;
		case 0xB9: std::cout << " " << HEX2X(operand1) << "   "                  << " LDA 0x00" << HEX2X((uint8_t)(operand1 + register_y)) << " = " << HEX2X(Read((uint8_t)(operand1 + register_y))) << '\n'; break;
		case 0xBA: std::cout << "      "                                         << " TSX" << '\n'; break;
		case 0xBB: std::cout << "      "                                         << " Illegal Opcode " << HEX2(instruction) << '\n'; break;
		case 0xBC: std::cout << " " << HEX2X(operand1) << "   "                  << " LDY " << HEX4((operand2 << 8) + operand1 + register_x) << " = " << HEX2X(Read((operand2 << 8) + operand1 + register_x)) << '\n'; break;
		case 0xBD: std::cout << " " << HEX2X(operand1) << "   "                  << " LDA " << HEX4((operand2 << 8) + operand1 + register_x) << " = " << HEX2X(Read((operand2 << 8) + operand1 + register_x)) << '\n'; break;
		case 0xBE: std::cout << " " << HEX2X(operand1) << "   "                  << " LDX " << HEX4((operand2 << 8) + operand1 + register_y) << " = " << HEX2X(Read((operand2 << 8) + operand1 + register_y)) << '\n'; break;
		case 0xBF: std::cout << "      "                                         << " Illegal Opcode " << HEX2(instruction) << '\n'; break;
		case 0xC0: std::cout << " " << HEX2X(operand1) << "   "                  << " CPY " << HEX2X(operand1) << '\n'; break;
		case 0xC1: std::cout << " " << HEX2X(operand1) << "   "                  << " CMP ?INDX? " << HEX2X(operand1) << '\n'; break;
		case 0xC2: std::cout << "      "                                         << " Illegal Opcode " << HEX2(instruction) << '\n'; break;
		case 0xC3: std::cout << "      "                                         << " Illegal Opcode " << HEX2(instruction) << '\n'; break;
		case 0xC4: std::cout << " " << HEX2X(operand1) << "   "                  << " CPY 0x00" << HEX2X(operand1) << " = " << HEX2X(Read(operand1)) << '\n'; break;
		case 0xC5: std::cout << " " << HEX2X(operand1) << "   "                  << " CMP 0x00" << HEX2X(operand1) << " = " << HEX2X(Read(operand1)) << '\n'; break;
		case 0xC6: std::cout << " " << HEX2X(operand1) << "   "                  << " DEC 0x00" << HEX2X(operand1) << " = " << HEX2X(Read(operand1) - 1) << '\n'; break;
		case 0xC7: std::cout << "      "                                         << " Illegal Opcode " << HEX2(instruction) << '\n'; break;
		case 0xC8: std::cout << "      "                                         << " INY Y = " << HEX2((uint8_t) register_y + 1) << '\n'; break;
		case 0xC9: std::cout << " " << HEX2X(operand1) << "   "                  << " CMP " << HEX2(operand1) << '\n'; break;
		case 0xCA: std::cout << "      "                                         << " DEX X = " << HEX2(register_x - 1) << '\n'; break;
		case 0xCB: std::cout << "      "                                         << " Illegal Opcode " << HEX2(instruction) << '\n'; break;
		case 0xCC: std::cout << " " << HEX2X(operand1) << " " << HEX2X(operand2) << " CPY " << HEX4((operand2 << 8) + operand1) << " = " << HEX2X(Read((operand2 << 8) + operand1)) << '\n'; break;
		case 0xCD: std::cout << " " << HEX2X(operand1) << "   "                  << " CMP " << HEX4((operand2 << 8) + operand1) << " = " << HEX2X(Read((operand2 << 8) + operand1)) << '\n'; break;
		case 0xCE: std::cout << " " << HEX2X(operand1) << "   "                  << " DEC " << HEX4((operand2 << 8) + operand1) << " = " << HEX2X(Read((operand2 << 8) + operand1) - 1) << '\n'; break;
		case 0xCF: std::cout << "      "                                         << " Illegal Opcode " << HEX2(instruction) << '\n'; break;
		case 0xD0: std::cout << " " << HEX2X(operand1) << "   "                  << " BNE " << HEX4(program_counter + (int8_t)operand1 + 2) << '\n'; break;
		case 0xD1: std::cout << " " << HEX2X(operand1) << "   "                  << " CMP ?INDY?" << '\n'; break;
		case 0xD2: std::cout << "      "                                         << " Illegal Opcode " << HEX2(instruction) << '\n'; break;
		case 0xD3: std::cout << "      "                                         << " Illegal Opcode " << HEX2(instruction) << '\n'; break;
		case 0xD4: std::cout << "      "                                         << " Illegal Opcode " << HEX2(instruction) << '\n'; break;
		case 0xD5: std::cout << " " << HEX2X(operand1) << "   "                  << " CMP 0x00" << HEX2X((uint8_t)(operand1 + register_x)) << " = " << HEX2X(Read((uint8_t)(operand1 + register_x))) << '\n'; break;
		case 0xD6: std::cout << " " << HEX2X(operand1) << "   "                  << " DEC" << '\n'; break;
		case 0xD7: std::cout << "      "                                         << " Illegal Opcode " << HEX2(instruction) << '\n'; break;
		case 0xD8: std::cout << "      "                                         << " CLD" << '\n'; break;
		case 0xD9: std::cout << " " << HEX2X(operand1) << "   "                  << " CMP " << HEX4((operand2 << 8) + operand1 + register_y) << " = " << HEX2X(Read((operand2 << 8) + operand1 + register_y)) << '\n'; break;
		case 0xDA: std::cout << "      "                                         << " Illegal Opcode " << HEX2(instruction) << '\n'; break;
		case 0xDB: std::cout << "      "                                         << " Illegal Opcode " << HEX2(instruction) << '\n'; break;
		case 0xDC: std::cout << "      "                                         << " Illegal Opcode " << HEX2(instruction) << '\n'; break;
		case 0xDD: std::cout << " " << HEX2X(operand1) << "   "                  << " CMP " << HEX4((operand2 << 8) + operand1 + register_x) << " = " << HEX2X(Read((operand2 << 8) + operand1 + register_x)) << '\n'; break;
		case 0xDE: std::cout << " " << HEX2X(operand1) << "   "                  << " DEC " << HEX4((operand2 << 8) + operand1 + register_x) << " = " << HEX2X(Read((operand2 << 8) + operand1 + register_x) - 1) << '\n'; break;
		case 0xDF: std::cout << "      "                                         << " Illegal Opcode " << HEX2(instruction) << '\n'; break;
		case 0xE0: std::cout << " " << HEX2X(operand1) << "   "                  << " CPX " << HEX2X(operand1) << '\n'; break;
		case 0xE1: std::cout << " " << HEX2X(operand1) << "   "                  << " SBC ?INDY?" << '\n'; break;
		case 0xE2: std::cout << "      "                                         << " Illegal Opcode " << HEX2(instruction) << '\n'; break;
		case 0xE3: std::cout << "      "                                         << " Illegal Opcode " << HEX2(instruction) << '\n'; break;
		case 0xE4: std::cout << " " << HEX2X(operand1) << "   "                  << " CPX 0x00" << HEX2X(operand1) << " = " << HEX2X(Read(operand1)) << '\n'; break;
		case 0xE5: std::cout << " " << HEX2X(operand1) << "   "                  << " SBC 0x00" << HEX2X(operand1) << " = " << HEX2X(Read(operand1)) << '\n'; break;
		case 0xE6: std::cout << " " << HEX2X(operand1) << "   "                  << " INC 0x00" << HEX2X(operand1) << " = " << HEX2X(Read(operand1) + 1) << '\n'; break;
		case 0xE7: std::cout << "      "                                         << " Illegal Opcode " << HEX2(instruction) << '\n'; break;
		case 0xE8: std::cout << "      "                                         << " INX X = " << HEX2(register_x) << '\n'; break;
		case 0xE9: std::cout << " " << HEX2X(operand1) << "   "                  << " SBC " << HEX2X(operand1) << '\n'; break;
		case 0xEA: std::cout << "      "                                         << " NOP" << '\n'; break;
		case 0xEB: std::cout << "      "                                         << " Illegal Opcode " << HEX2(instruction) << '\n'; break;
		case 0xEC: std::cout << " " << HEX2X(operand1) << " " << HEX2X(operand2) << " CPX " << HEX4((operand2 << 8) + operand1) << " = " << HEX2X(Read((operand2 << 8) + operand1)) << '\n'; break;
		case 0xED: std::cout << " " << HEX2X(operand1) << " " << HEX2X(operand2) << " SBC " << HEX4((operand2 << 8) + operand1) << " = " << HEX2X(Read((operand2 << 8) + operand1)) << '\n'; break;
		case 0xEE: std::cout << " " << HEX2X(operand1) << " " << HEX2X(operand2) << " INC " << HEX4((operand2 << 8) + operand1) << " = " << HEX2X(Read((operand2 << 8) + operand1) + 1) << '\n'; break;
		case 0xEF: std::cout << "      "                                         << " Illegal Opcode " << HEX2(instruction) << '\n'; break;
		case 0xF0: std::cout << " " << HEX2X(operand1) << "   "                  << " BEQ " << HEX4((program_counter + (int8_t)operand1 + 2)) << '\n'; break;
		case 0xF1: std::cout << " " << HEX2X(operand1) << "   "                  << " SBC ?INDY?" << '\n'; break;
		case 0xF2: std::cout << "      "                                         << " Illegal Opcode " << HEX2(instruction) << '\n'; break;
		case 0xF3: std::cout << "      "                                         << " Illegal Opcode " << HEX2(instruction) << '\n'; break;
		case 0xF4: std::cout << "      "                                         << " Illegal Opcode " << HEX2(instruction) << '\n'; break;
		case 0xF5: std::cout << " " << HEX2X(operand1) << "   "                  << " SBC ZPGX" << '\n'; break;
		case 0xF6: std::cout << " " << HEX2X(operand1) << "   "                  << " INC ZPGX" << '\n'; break;
		case 0xF7: std::cout << "      "                                         << " Illegal Opcode " << HEX2(instruction) << '\n'; break;
		case 0xF8: std::cout << "      "                                         << " SED" << '\n'; break;
		case 0xF9: std::cout << " " << HEX2X(operand1) << " " << HEX2X(operand2) << " SBC " << HEX4((operand2 << 8) + operand1 + register_y) << '\n'; break;
		case 0xFA: std::cout << "      "                                         << " Illegal Opcode " << HEX2(instruction) << '\n'; break;
		case 0xFB: std::cout << "      "                                         << " Illegal Opcode " << HEX2(instruction) << '\n'; break;
		case 0xFC: std::cout << "      "                                         << " Illegal Opcode " << HEX2(instruction) << '\n'; break;
		case 0xFD: std::cout << " " << HEX2X(operand1) << " " << HEX2X(operand2) << " SBC " << HEX4((operand2 << 8) + operand1 + register_x) << '\n'; break;
		case 0xFE: std::cout << "      "                                         << " Illegal Opcode " << HEX2(instruction) << '\n'; break;
		case 0xFF: std::cout << "      "                                         << " Illegal Opcode " << HEX2(instruction) << '\n'; break;
		default:
			std::cout << "Disassembly not written for: " << HEX2(instruction) << '\n';
			break;
	}
}