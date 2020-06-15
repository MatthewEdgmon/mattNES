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

#include <SDL.h>

#include "../BitOps.hpp"
#include "../HexOutput.hpp"

#include "CPU.hpp"
#include "NESSystem.hpp"

void CPU::IllegalOpcode(uint8_t opcode) {
	std::cout << "Caught Illegal Opcode " << HEX2(opcode) << " at " << HEX4(program_counter) << '\n';
	std::cout << "Dump Info Below" << '\n';
	nes_system->DumpTestInfo();
	while (1) {
		SDL_Delay(1);
	}
}

void CPU::Step() {

	if(halted) {
		return;
	}

	instruction = Read(program_counter);

	uint16_t target = 0;
	uint8_t result = 0;

	if(show_disassembly) {
		operand1 = ReadDebug(program_counter + 1);
		operand2 = ReadDebug(program_counter + 2);
		StepDisassembler();
	}

	switch(instruction) {
		case 0x00:
			program_counter += 2;
			/* Push low byte, high byte, and flags in that order. */
			Push(program_counter &  0xFF);
			Push(program_counter >> 8);
			Push(register_p);
			BitSet(register_p, STATUS_BIT_INTERRUPT_DISABLE);
			program_counter = vector_irq;
			increment_pc = false;
			cycles += 7;
			break;
		case 0x01:
			/* Indirect X Index added here to force wrap around instead of carry in the line after. */
			operand1 = (Read(program_counter + 1) + register_x);
			std::cout << "ORA INDX unimp" << '\n';
			program_counter += 1;
			cycles += 6;
			break;
		case 0x02: IllegalOpcode(0x02);
		case 0x03: IllegalOpcode(0x03);
		case 0x04: IllegalOpcode(0x04);
		case 0x05:
			operand1 = Read(program_counter + 1);
			operand1 = Read(operand1);
			register_a |= operand1;
			if(register_a == 0x00)     { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
            if(register_a >= 0x80)     { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 1;
			cycles += 3;
			break;
		case 0x06:
			operand1 = Read(program_counter + 1);
			operand2 = Read(operand1);
			/* Check if bit 7 is set to preserve value. */
			if(operand2 & 0x80) {
				BitSet(register_p, STATUS_BIT_CARRY);
			} else {
				BitClear(register_p, STATUS_BIT_CARRY);
			}
			operand2 <<= 1;
			Write(operand1, operand2);
			if(operand2 == 0x00)       { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(operand2 >= 0x80)       { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 1;
			cycles += 5;
			break;
		case 0x07: IllegalOpcode(0x07);
		case 0x08:
			Push(register_p);
			cycles += 3;
			break;
		case 0x09:
			operand1 = Read(program_counter + 1);
			register_a |= operand1;
            if(register_a == 0x00)     { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
            if(register_a >= 0x80)     { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 1;
			cycles += 2;
			break;
		case 0x0A:
			/* Check if bit 7 is set to preserve value. */
			if(register_a & 0x80) {
				BitSet(register_p, STATUS_BIT_CARRY);
			} else {
				BitClear(register_p, STATUS_BIT_CARRY);
			}
			register_a <<= 1;
			if(register_a == 0x00)     { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(register_a >= 0x80)     { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			cycles += 5;
			break;
		case 0x0B: IllegalOpcode(0x0B);
		case 0x0C: IllegalOpcode(0x0C);
		case 0x0D:
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			register_a |= Read((operand2 << 8) + operand1);
            if(register_a == 0x00)     { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
            if(register_a >= 0x80)     { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 2;
			cycles += 4;
			break;
		case 0x0E:
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			result = Read((operand2 << 8) + operand1);
			/* Check if bit 7 is set to preserve value. */
			if(result & 0x80) {
				BitSet(register_p, STATUS_BIT_CARRY);
			} else {
				BitClear(register_p, STATUS_BIT_CARRY);
			}
			result <<= 1;
			Write((operand2 << 8) + operand1, result);
			if(result == 0x00)     { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(result >= 0x80)     { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 2;
			cycles += 6;
			break;
		case 0x0F: IllegalOpcode(0x0F);
		case 0x10:
			operand1 = Read(program_counter + 1);
			program_counter++;
			if(BitCheck(register_p, STATUS_BIT_NEGATIVE) == 0) {
				// TODO: Add extra cycle if crossing page boundary (0x100).
				program_counter += (int8_t)operand1;
				cycles += 1;
			}
			cycles += 2;
			break;
		case 0x11:
			std::cout << "ORA INDY unimp" << '\n';
			program_counter += 1;
			cycles += 2;
			break;
		case 0x12: IllegalOpcode(0x12);
		case 0x13: IllegalOpcode(0x13);
		case 0x14: IllegalOpcode(0x14);
		case 0x15:
			/* Zero Page X Index added here to force wrap around instead of carry in the line after. */
			operand1 = (Read(program_counter + 1) + register_x);
			operand1 = Read(operand1);
			register_a |= operand1;
            if(register_a == 0x00)     { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
            if(register_a >= 0x80)     { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 1;
			cycles += 4;
			break;
		case 0x16:
			/* Zero Page X Index added here to force wrap around instead of carry in the line after. */
			operand1 = (Read(program_counter + 1) + register_x);
			operand2 = Read(operand1);
			/* Check if bit 7 is set to preserve value. */
			if(operand2 & 0x80) {
				BitSet(register_p, STATUS_BIT_CARRY);
			} else {
				BitClear(register_p, STATUS_BIT_CARRY);
			}
			operand2 <<= 1;
			Write(operand1, operand2);
			if(operand2 == 0x00)       { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(operand2 >= 0x80)       { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 1;
			cycles += 6;
			break;
		case 0x17: IllegalOpcode(0x17);
		case 0x18:
			BitClear(register_p, STATUS_BIT_CARRY);
			cycles += 2;
			break;
		case 0x19:
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			register_a |= Read((operand2 << 8) + operand1 + register_y);
            if(register_a == 0x00)     { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
            if(register_a >= 0x80)     { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 1;
			cycles += 4; // + 1 for page
			break;
		case 0x1A: IllegalOpcode(0x1A);
		case 0x1B: IllegalOpcode(0x1B);
		case 0x1C: IllegalOpcode(0x1C);
		case 0x1D:
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			register_a |= Read((operand2 << 8) + operand1 + register_x);
            if(register_a == 0x00)     { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
            if(register_a >= 0x80)     { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 1;
			cycles += 4; // + 1 for page
			break;
		case 0x1E:
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			result = Read((operand2 << 8) + operand1 + register_x);
			/* Check if bit 7 is set to preserve value. */
			if(result & 0x80) {
				BitSet(register_p, STATUS_BIT_CARRY);
			} else {
				BitClear(register_p, STATUS_BIT_CARRY);
			}
			result <<= 1;
			Write((operand2 << 8) + operand1 + register_x, result);
			if(result == 0x00)     { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(result >= 0x80)     { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 2;
			cycles += 6;
			break;
		case 0x1F: IllegalOpcode(0x1F);
		case 0x20:
			/* Set the return address three bytes ahead of current PC. TODO: Implement accurate FDE to make PC only plus two. */
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			program_counter += 3; 
			Push(program_counter & 0xFF);
			Push(program_counter >> 8);
			program_counter = (operand2 << 8) + operand1;
			increment_pc = false;
			cycles += 6;
			break;
		case 0x21:
			std::cout << "AND INDX unimp" << '\n';
			program_counter += 1;
			cycles += 6;
			break;
		case 0x22: IllegalOpcode(0x22);
		case 0x23: IllegalOpcode(0x23);
		case 0x24:
			operand1 = Read(program_counter + 1);
			operand1 = Read(operand1);
			if((operand1 & register_a) == 0) { BitSet(register_p, STATUS_BIT_ZERO); } else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(BitCheck(operand1, 6))  { BitSet(register_p, STATUS_BIT_OVERFLOW); } else { BitClear(register_p, STATUS_BIT_OVERFLOW); }
			if(BitCheck(operand1, 7))  { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 1;
			cycles += 3;
			break;
		case 0x25:
			operand1 = Read(program_counter + 1);
			operand2 = Read(operand1);
			register_a &= operand2;
			if(register_a == 0x00)     { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(register_a >= 0x80)     { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 1;
			cycles += 3;
			break;
		case 0x26:
			operand1 = Read(program_counter + 1);
			operand2 = Read(operand1);
			/* Check if carry flag is set to preserve value. */
			if(BitCheck(register_p, STATUS_BIT_CARRY)) {
				/* Yes, add 0x01 at end to preserve old carry flag. */
				/* Check if bit 7 is set to preserve value. */
				if(operand2 & 0x80) {
					/* Yes, set carry flag to preserve bit 7. */
					BitSet(register_p, STATUS_BIT_CARRY);
				} else {
					/* No, clear carry flag to preserve bit 7. */
					BitClear(register_p, STATUS_BIT_CARRY);
				}
				/* Perform shift, add old bit 1. */
				operand2 = (operand2 << 1) + 0x01;
			} else {
				/* No, add nothing and let bit 0 go to 0 to preserve carry flag. */
				/* Check if bit 7 is set to preserve value. */
				if(operand2 & 0x80) {
					/* Yes, set carry flag to preserve bit 7. */
					BitSet(register_p, STATUS_BIT_CARRY);
				} else {
					/* No, clear carry flag to preserve bit 7. */
					BitClear(register_p, STATUS_BIT_CARRY);
				}
				/* Perform shift, add nothing. */
				operand2 = (operand2 << 1);
			}
			/* Finally write value. */
			Write(operand1, operand2);
			if(operand2 == 0x00)       { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(operand2 >= 0x80)       { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 1;
			cycles += 5;
			break;
		case 0x27: IllegalOpcode(0x27);
		case 0x28:
			register_p = Pop();
			cycles += 4;
			break;
		case 0x29:
			operand1 = Read(program_counter + 1);
			register_a &= operand1;
			if(register_a == 0x00)     { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(register_a >= 0x80)     { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 1;
			cycles += 2;
			break;
		case 0x2A:
			/* Check if carry flag is set to preserve value. */
			if(BitCheck(register_p, STATUS_BIT_CARRY)) {
				/* Yes, add 0x01 at end to preserve old carry flag. */
				/* Check if bit 7 is set to preserve value. */
				if(register_a & 0x80) {
					/* Yes, set carry flag to preserve bit 7. */
					BitSet(register_p, STATUS_BIT_CARRY);
				} else {
					/* No, clear carry flag to preserve bit 7. */
					BitClear(register_p, STATUS_BIT_CARRY);
				}
				/* Perform shift, add old bit 1. */
				register_a = (register_a << 1) + 0x01;
			} else {
				/* No, add nothing and let bit 0 go to 0 to preserve carry flag. */
				/* Check if bit 7 is set to preserve value. */
				if(register_a & 0x80) {
					/* Yes, set carry flag to preserve bit 7. */
					BitSet(register_p, STATUS_BIT_CARRY);
				} else {
					/* No, clear carry flag to preserve bit 7. */
					BitClear(register_p, STATUS_BIT_CARRY);
				}
				/* Perform shift, add nothing. */
				register_a = (register_a << 1);
			}
			if(register_a == 0x00)     { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(register_a >= 0x80)     { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			cycles += 2;
			break;
		case 0x2B: IllegalOpcode(0x2B);
		case 0x2C:
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			operand1 = Read((operand2 << 8) + operand1);
			if((operand1 & register_a) == 0) { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(BitCheck(operand1, 6))        { BitSet(register_p, STATUS_BIT_OVERFLOW); } else { BitClear(register_p, STATUS_BIT_OVERFLOW); }
			if(BitCheck(operand1, 7))        { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 2;
			cycles += 4;
			break;
		case 0x2D:
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			register_a &= Read((operand2 << 8) + operand1);
			if(register_a == 0x00)     { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(register_a >= 0x80)     { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 2;
			cycles += 4;
			break;
		case 0x2E:
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			result = Read((operand2 << 8) + operand1);
			/* Check if carry flag is set to preserve value. */
			if(BitCheck(register_p, STATUS_BIT_CARRY)) {
				/* Yes, add 0x01 at end to preserve old carry flag. */
				/* Check if bit 7 is set to preserve value. */
				if(result & 0x80) {
					/* Yes, set carry flag to preserve bit 7. */
					BitSet(register_p, STATUS_BIT_CARRY);
				} else {
					/* No, clear carry flag to preserve bit 7. */
					BitClear(register_p, STATUS_BIT_CARRY);
				}
				/* Perform shift, add old bit 1. */
				result = (result << 1) + 0x01;
			} else {
				/* No, add nothing and let bit 0 go to 0 to preserve carry flag. */
				/* Check if bit 7 is set to preserve value. */
				if(result & 0x80) {
					/* Yes, set carry flag to preserve bit 7. */
					BitSet(register_p, STATUS_BIT_CARRY);
				} else {
					/* No, clear carry flag to preserve bit 7. */
					BitClear(register_p, STATUS_BIT_CARRY);
				}
				/* Perform shift, add nothing. */
				result = (result << 1);
			}
			Write((operand2 << 8) + operand1, result);
			if(result == 0x00)         { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(result >= 0x80)         { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 2;
			cycles += 6;
			break;
		case 0x2F: IllegalOpcode(0x2F);
		case 0x30:
			operand1 = Read(program_counter + 1);
			program_counter++;
			if(BitCheck(register_p, STATUS_BIT_NEGATIVE)) {
				// TODO: Add extra cycle if crossing page boundary (0x100).
				program_counter += (int8_t)operand1;
				cycles += 1; // +2 if page
			}
			cycles += 2;
			break;
		case 0x31:
			std::cout << "AND INDY unimp" << '\n';
			program_counter += 1;
			cycles += 5;
			break;
		case 0x32: IllegalOpcode(0x32);
		case 0x33: IllegalOpcode(0x33);
		case 0x34: IllegalOpcode(0x34);
		case 0x35:
			/* Zero Page X Index added here to force wrap around instead of carry in the line after. */
			operand1 = Read(program_counter + 1) + register_x;
			operand2 = Read(operand1);
			register_a &= operand2;
			if(register_a == 0x00)     { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(register_a >= 0x80)     { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 1;
			cycles += 4;
			break;
		case 0x36:
			/* Zero Page X Index added here to force wrap around instead of carry in the line after. */
			operand1 = Read(program_counter + 1) + register_x;
			operand2 = Read(operand1);
			/* Check if carry flag is set to preserve value. */
			if(BitCheck(register_p, STATUS_BIT_CARRY)) {
				/* Yes, add 0x01 at end to preserve old carry flag. */
				/* Check if bit 7 is set to preserve value. */
				if(operand2 & 0x80) {
					/* Yes, set carry flag to preserve bit 7. */
					BitSet(register_p, STATUS_BIT_CARRY);
				} else {
					/* No, clear carry flag to preserve bit 7. */
					BitClear(register_p, STATUS_BIT_CARRY);
				}
				/* Perform shift, add old bit 1. */
				operand2 = (operand2 << 1) + 0x01;
			} else {
				/* No, add nothing and let bit 0 go to 0 to preserve carry flag. */
				/* Check if bit 7 is set to preserve value. */
				if(operand2 & 0x80) {
					/* Yes, set carry flag to preserve bit 7. */
					BitSet(register_p, STATUS_BIT_CARRY);
				} else {
					/* No, clear carry flag to preserve bit 7. */
					BitClear(register_p, STATUS_BIT_CARRY);
				}
				/* Perform shift, add nothing. */
				operand2 = (operand2 << 1);
			}
			Write(operand1, operand2);
			if(operand2 == 0x00)       { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(operand2 >= 0x80)       { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 1;
			cycles += 6;
			break;
		case 0x37: IllegalOpcode(0x37);
		case 0x38:
			BitSet(register_p, STATUS_BIT_CARRY);
			cycles += 2;
			break;
		case 0x39:
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			register_a &= Read((operand2 << 8) + operand1 + register_y);
			if(register_a == 0x00)     { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(register_a >= 0x80)     { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 2;
			cycles += 4; // + 1 for page
			break;
		case 0x3A: IllegalOpcode(0x3A);
		case 0x3B: IllegalOpcode(0x3B);
		case 0x3C: IllegalOpcode(0x3C);
		case 0x3D:
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			register_a &= Read((operand2 << 8) + operand1 + register_x);
			if(register_a == 0x00)     { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(register_a >= 0x80)     { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 2;
			cycles += 4; // + 1 for page
			break;
		case 0x3E:
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			result = Read((operand2 << 8) + operand1 + register_x);
			/* Check if carry flag is set to preserve value. */
			if(BitCheck(register_p, STATUS_BIT_CARRY)) {
				/* Yes, add 0x01 at end to preserve old carry flag. */
				/* Check if bit 7 is set to preserve value. */
				if(result & 0x80) {
					/* Yes, set carry flag to preserve bit 7. */
					BitSet(register_p, STATUS_BIT_CARRY);
				} else {
					/* No, clear carry flag to preserve bit 7. */
					BitClear(register_p, STATUS_BIT_CARRY);
				}
				/* Perform shift, add old bit 1. */
				result = (result << 1) + 0x01;
			} else {
				/* No, add nothing and let bit 0 go to 0 to preserve carry flag. */
				/* Check if bit 7 is set to preserve value. */
				if(result & 0x80) {
					/* Yes, set carry flag to preserve bit 7. */
					BitSet(register_p, STATUS_BIT_CARRY);
				} else {
					/* No, clear carry flag to preserve bit 7. */
					BitClear(register_p, STATUS_BIT_CARRY);
				}
				/* Perform shift, add nothing. */
				result = (result << 1);
			}
			Write(((operand2 << 8) + operand1 + register_x), result);
			if(result == 0x00)         { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(result >= 0x80)         { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 2;
			cycles += 6;
			break;
		case 0x3F: IllegalOpcode(0x3F);
		case 0x40:
			/* Pop flags, high byte, and low byte in that order. */
			register_p = Pop();
			program_counter  = Pop() << 8;
			program_counter += Pop();
			increment_pc = false;
			cycles += 6;
			break;
		case 0x41:
			std::cout << "ROL INDX unimp" << '\n';
			program_counter += 1;
			cycles += 6;
			break;
		case 0x42: IllegalOpcode(0x42);
		case 0x43: IllegalOpcode(0x43);
		case 0x44: IllegalOpcode(0x44);
		case 0x45:
			operand1 = Read(program_counter + 1);
			operand1 = Read(operand1);
			register_a ^= operand1;
			if(register_a == 0x00)     { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(register_a >= 0x80)     { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 1;
			cycles += 3;
			break;
		case 0x46:
			operand1 = Read(program_counter + 1);
			operand2 = Read(operand1);
			/* Check if bit 7 is set to preserve value. */
			if(operand2 & 0x1) {
				BitSet(register_p, STATUS_BIT_CARRY);
			} else {
				BitClear(register_p, STATUS_BIT_CARRY);
			}
			operand2 >>= 1;
			Write(operand1, operand2);
			if(operand2 == 0x00)       { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(operand2 >= 0x80)       { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 1;
			cycles += 5;
			break;
		case 0x47: IllegalOpcode(0x47);
		case 0x48:
			Push(register_a);
			cycles += 3;
			break;
		case 0x49:
			operand1 = Read(program_counter + 1);
			register_a ^= operand1;
			if(register_a == 0x00)     { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(register_a >= 0x80)     { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 1;
			cycles += 2;
			break;
		case 0x4A:
			/* Check if bit 7 is set to preserve value. */
			if(register_a & 0x01) {
				BitSet(register_p, STATUS_BIT_CARRY);
			} else {
				BitClear(register_p, STATUS_BIT_CARRY);
			}
			register_a >>= 1;
			if(register_a == 0x00)     { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(register_a >= 0x80)     { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			cycles += 2;
			break;
		case 0x4B: IllegalOpcode(0x4B);
		case 0x4C:
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			program_counter = (operand2 << 8) + operand1;
			increment_pc = false;
			cycles += 3;
			break;
		case 0x4D:
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			register_a ^= Read((operand2 << 8) + operand1);
			if(register_a == 0x00)     { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(register_a >= 0x80)     { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 2;
			cycles += 4;
			break;
		case 0x4E:
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			result = Read((operand2 << 8) + operand1);
			/* Check if bit 7 is set to preserve value. */
			if(result & 0x1) {
				BitSet(register_p, STATUS_BIT_CARRY);
			} else {
				BitClear(register_p, STATUS_BIT_CARRY);
			}
			result >>= 1;
			Write((operand2 << 8) + operand1, result);
			if(result == 0x00)       { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(result >= 0x80)       { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 2;
			cycles += 6;
			break;
		case 0x4F: IllegalOpcode(0x4F);
		case 0x50:
			operand1 = Read(program_counter + 1);
			program_counter++;
			if(!BitCheck(register_p, STATUS_BIT_OVERFLOW)) {
				// TODO: Add extra cycle if crossing page boundary (0x100).
				program_counter += (int8_t)operand1;
				cycles += 1; // +2 if page
			}
			cycles += 2;
			break;
		case 0x51:
			std::cout << "EOR INDY unimp" << '\n';
			program_counter += 1;
			cycles += 5; // + 1 for page
			break;
		case 0x52: IllegalOpcode(0x52);
		case 0x53: IllegalOpcode(0x53);
		case 0x54: IllegalOpcode(0x54);
		case 0x55:
			/* Zero Page X Index added here to force wrap around instead of carry in the line after. */
			operand1 = Read(program_counter + 1) + register_x;
			operand1 = Read(operand1);
			register_a ^= operand1;
			if(register_a == 0x00)     { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(register_a >= 0x80)     { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 1;
			cycles += 4;
			break;
		case 0x56:
			/* Zero Page X Index added here to force wrap around instead of carry in the line after. */
			operand1 = Read(program_counter + 1) + register_x;
			operand2 = Read(operand1);
			/* Check if bit 7 is set to preserve value. */
			if(operand2 & 0x1) {
				BitSet(register_p, STATUS_BIT_CARRY);
			} else {
				BitClear(register_p, STATUS_BIT_CARRY);
			}
			operand2 >>= 1;
			Write(operand1, operand2);
			if(operand2 == 0x00)       { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(operand2 >= 0x80)       { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 1;
			cycles += 6;
			break;
		case 0x57: IllegalOpcode(0x57);
		case 0x58:
			BitClear(register_p, STATUS_BIT_INTERRUPT_DISABLE);
			cycles += 2;
			break;
		case 0x59:
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			register_a ^= Read((operand2 << 8) + operand1 + register_y);
			if(register_a == 0x00)     { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(register_a >= 0x80)     { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 2;
			cycles += 4; // + 1 for page
			break;
		case 0x5A: IllegalOpcode(0x5A);
		case 0x5B: IllegalOpcode(0x5B);
		case 0x5C: IllegalOpcode(0x5C);
		case 0x5D:
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			register_a ^= Read((operand2 << 8) + operand1 + register_x);
			if(register_a == 0x00)     { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(register_a >= 0x80)     { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 2;
			cycles += 4; // + 1 for page
			break;
		case 0x5E:
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			result = Read((operand2 << 8) + operand1 + register_x);
			/* Check if bit 7 is set to preserve value. */
			if(result & 0x1) {
				BitSet(register_p, STATUS_BIT_CARRY);
			} else {
				BitClear(register_p, STATUS_BIT_CARRY);
			}
			result >>= 1;
			Write(((operand2 << 8) + operand1 + register_x), result);
			if(result == 0x00)         { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(result >= 0x80)         { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 2;
			cycles += 7;
			break;
		case 0x5F: IllegalOpcode(0x5F);
		case 0x60:
			operand2 = Pop();
			operand1 = Pop();
			program_counter = ((operand2 << 8) + operand1);
			increment_pc = false;
			cycles += 6;
			break;
		case 0x61:
			std::cout << "ADC INDX unimp" << '\n';
			program_counter += 1;
			cycles += 6;
			break;
		case 0x62: IllegalOpcode(0x62);
		case 0x63: IllegalOpcode(0x63);
		case 0x64: IllegalOpcode(0x64);
		case 0x65:
			operand1 = Read(program_counter + 1);
			operand1 = Read(operand1);
			register_a = register_a + operand1 + BitCheck(register_p, STATUS_BIT_CARRY);
			// TODO: Check for carry   { BitSet(register_p, STATUS_BIT_CARRY); }    else { BitClear(register_p, STATUS_BIT_CARRY); }
			if(register_a == 0x00)     { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			// TODO: Check for overflow{ BitSet(register_p, STATUS_BIT_OVERFLOW); } else { BitClear(register_p, STATUS_BIT_OVERFLOW); }
			if(register_a >= 0x80)     { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 1;
			cycles += 3;
			break;
		case 0x66:
			operand1 = Read(program_counter + 1);
			operand2 = Read(operand1);
			/* Check if carry flag is set to preserve value. */
			if(BitCheck(register_p, STATUS_BIT_CARRY)) {
				/* Yes, add 0x80 at end to preserve old carry flag. */
				/* Check if bit 0 is set to preserve value. */
				if(operand2 & 0x01) {
					/* Yes, set carry flag to preserve bit 0. */
					BitSet(register_p, STATUS_BIT_CARRY);
				} else {
					/* No, clear carry flag to preserve bit 0. */
					BitClear(register_p, STATUS_BIT_CARRY);
				}
				/* Perform shift, add old bit 7. */
				operand2 = (operand2 >> 1) + 0x80;
			} else {
				/* No, add nothing and let bit 7 go to 0. */
				/* Check if bit 0 is set to preserve value. */
				if(operand2 & 0x01) {
					/* Yes, set carry flag to preserve bit 0. */
					BitSet(register_p, STATUS_BIT_CARRY);
				} else {
					/* No, clear carry flag to preserve bit 0. */
					BitClear(register_p, STATUS_BIT_CARRY);
				}
				/* Perform shift, add nothing. */
				operand2 = (operand2 >> 1);
			}
			/* Finally write value. */
			Write(operand1, operand2);
			if(operand2 == 0x00)       { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(operand2 >= 0x80)       { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 1;
			cycles += 5;
			break;
		case 0x67: IllegalOpcode(0x67);
		case 0x68:
			register_a = Pop();
			if(register_a == 0x00)     { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(register_a >= 0x80)     { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			cycles += 4;
			break;
		case 0x69:
			operand1 = Read(program_counter + 1);
			register_a = register_a + operand1 + BitCheck(register_p, STATUS_BIT_CARRY);
			// TODO: Check for carry   { BitSet(register_p, STATUS_BIT_CARRY); }    else { BitClear(register_p, STATUS_BIT_CARRY); }
			if(register_a == 0x00)     { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			// TODO: Check for overflow{ BitSet(register_p, STATUS_BIT_OVERFLOW); } else { BitClear(register_p, STATUS_BIT_OVERFLOW); }
			if(register_a >= 0x80)     { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 1;
			cycles += 2;
			break;
		case 0x6A:
			/* Check if carry flag is set to preserve value. */
			if(BitCheck(register_p, STATUS_BIT_CARRY)) {
				/* Yes, add 0x01 at end to preserve old carry flag. */
				/* Check if bit 7 is set to preserve value. */
				if(register_a & 0x80) {
					/* Yes, set carry flag to preserve bit 7. */
					BitSet(register_p, STATUS_BIT_CARRY);
				} else {
					/* No, clear carry flag to preserve bit 7. */
					BitClear(register_p, STATUS_BIT_CARRY);
				}
				/* Perform shift, add old bit 1. */
				register_a = (register_a << 1) + 0x01;
			} else {
				/* No, add nothing and let bit 0 go to 0 to preserve carry flag. */
				/* Check if bit 7 is set to preserve value. */
				if(register_a & 0x80) {
					/* Yes, set carry flag to preserve bit 7. */
					BitSet(register_p, STATUS_BIT_CARRY);
				} else {
					/* No, clear carry flag to preserve bit 7. */
					BitClear(register_p, STATUS_BIT_CARRY);
				}
				/* Perform shift, add nothing. */
				register_a = (register_a << 1);
			}
			if(register_a == 0x00)     { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(register_a >= 0x80)     { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			cycles += 2;
			break;
		case 0x6B: IllegalOpcode(0x6B);
		case 0x6C:
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			/* Bug on NMOS 6502: JMP ignores page crossing on page barrier (0xXXFF). */
			if(operand1 == 0xFF) {
				/* Ignore LSB, address fetched is from low byte 0x10FF and high byte 0x1000. */
				program_counter  = Read((operand2 << 8) + operand1);
				program_counter += Read((operand2 << 8)) << 8;
			} else {
				/* Perform expected behaviour. */
				program_counter  = Read((operand2 << 8) + operand1);
				program_counter += Read((operand2 << 8) + operand1 + 1) << 8;
			}
			increment_pc = false;
			cycles += 3;
			break;
		case 0x6D:
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			register_a += Read((operand2 << 8) + operand1) + BitCheck(register_p, STATUS_BIT_CARRY);
			// TODO: Check for carry   { BitSet(register_p, STATUS_BIT_CARRY); }    else { BitClear(register_p, STATUS_BIT_CARRY); }
			if(register_a == 0x00)     { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			// TODO: Check for overflow{ BitSet(register_p, STATUS_BIT_OVERFLOW); } else { BitClear(register_p, STATUS_BIT_OVERFLOW); }
			if(register_a >= 0x80)     { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 2;
			cycles += 4;
			break;
		case 0x6E:
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			result = Read((operand2 << 8) + operand1);
			/* Check if carry flag is set to preserve value. */
			if(BitCheck(register_p, STATUS_BIT_CARRY)) {
				/* Yes, add 0x80 at end to preserve old carry flag. */
				/* Check if bit 0 is set to preserve value. */
				if(result & 0x01) {
					/* Yes, set carry flag to preserve bit 0. */
					BitSet(register_p, STATUS_BIT_CARRY);
				} else {
					/* No, clear carry flag to preserve bit 0. */
					BitClear(register_p, STATUS_BIT_CARRY);
				}
				/* Perform shift, add bit from carry flag. */
				result = (result >> 1) + 0x80;
			} else {
				/* No, add nothing at end to preserve old carry flag. */
				/* Check if bit 0 is set to preserve value. */
				if(result & 0x01) {
					/* Yes, set carry flag to preserve bit 0. */
					BitSet(register_p, STATUS_BIT_CARRY);
				} else {
					/* No, clear carry flag to preserve bit 0. */
					BitClear(register_p, STATUS_BIT_CARRY);
				}
				/* Perform shift, add nothing. */
				result = (result >> 1);
			}
			Write((operand2 << 8) + operand1, result);
			if(result == 0x00)         { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(result >= 0x80)         { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 2;
			cycles += 6;
			break;
		case 0x6F: IllegalOpcode(0x6F);
		case 0x70:
			operand1 = Read(program_counter + 1);
			program_counter++;
			if(BitCheck(register_p, STATUS_BIT_OVERFLOW)) {
				// TODO: Add extra cycle if crossing page boundary (0x100).
				program_counter += (int8_t)operand1;
				cycles += 1;
			}
			cycles += 2;
			break;
		case 0x71:
			std::cout << "ADC INDY unimp" << '\n';
			program_counter += 1;
			cycles += 5; // +1 for page
			break;
		case 0x72: IllegalOpcode(0x72);
		case 0x73: IllegalOpcode(0x73);
		case 0x74: IllegalOpcode(0x74);
		case 0x75:
			/* Zero Page X Index added here to force wrap around instead of carry in the line after. */
			operand1 = Read(program_counter + 1) + register_x;
			operand1 = Read(operand1);
			register_a = register_a + operand1 + BitCheck(register_p, STATUS_BIT_CARRY);
			// TODO: Check for carry   { BitSet(register_p, STATUS_BIT_CARRY); }    else { BitClear(register_p, STATUS_BIT_CARRY); }
			if(register_a == 0x00)     { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			// TODO: Check for overflow{ BitSet(register_p, STATUS_BIT_OVERFLOW); } else { BitClear(register_p, STATUS_BIT_OVERFLOW); }
			if(register_a >= 0x80)     { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 1;
			cycles += 4;
			break;
		case 0x76:
			/* Zero Page X Index added here to force wrap around instead of carry in the line after. */
			operand1 = Read(program_counter + 1) + register_x;
			operand2 = Read(operand1);
			/* Check if carry flag is set to preserve value. */
			if(BitCheck(register_p, STATUS_BIT_CARRY)) {
				/* Yes, add 0x80 at end to preserve old carry flag. */
				/* Check if bit 0 is set to preserve value. */
				if(operand2 & 0x01) {
					/* Yes, set carry flag to preserve bit 0. */
					BitSet(register_p, STATUS_BIT_CARRY);
				} else {
					/* No, clear carry flag to preserve bit 0. */
					BitClear(register_p, STATUS_BIT_CARRY);
				}
				/* Perform shift, add bit from carry flag. */
				operand2 = (operand2 >> 1) + 0x80;
			} else {
				/* No, add nothing at end to preserve old carry flag. */
				/* Check if bit 0 is set to preserve value. */
				if(operand2 & 0x01) {
					/* Yes, set carry flag to preserve bit 0. */
					BitSet(register_p, STATUS_BIT_CARRY);
				} else {
					/* No, clear carry flag to preserve bit 0. */
					BitClear(register_p, STATUS_BIT_CARRY);
				}
				/* Perform shift, add nothing. */
				operand2 = (operand2 >> 1);
			}
			Write(operand1, operand2);
			if(operand2 == 0x00)       { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(operand2 >= 0x80)       { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 1;
			cycles += 6;
			break;
		case 0x77: IllegalOpcode(0x77);
		case 0x78:
			BitSet(register_p, STATUS_BIT_INTERRUPT_DISABLE);
			cycles += 2;
			break;
		case 0x79:
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			register_a += Read((operand2 << 8) + operand1 + register_y) + BitCheck(register_p, STATUS_BIT_CARRY);
			// TODO: Check for carry   { BitSet(register_p, STATUS_BIT_CARRY); }    else { BitClear(register_p, STATUS_BIT_CARRY); }
			if(register_a == 0x00)     { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			// TODO: Check for overflow{ BitSet(register_p, STATUS_BIT_OVERFLOW); } else { BitClear(register_p, STATUS_BIT_OVERFLOW); }
			if(register_a >= 0x80)     { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 2;
			cycles += 4; // + 1 for page
			break;
		case 0x7A: IllegalOpcode(0x7A);
		case 0x7B: IllegalOpcode(0x7B);
		case 0x7C: IllegalOpcode(0x7C);
		case 0x7D:
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			register_a += Read((operand2 << 8) + operand1 + register_x) + BitCheck(register_p, STATUS_BIT_CARRY);
			// TODO: Check for carry   { BitSet(register_p, STATUS_BIT_CARRY); }    else { BitClear(register_p, STATUS_BIT_CARRY); }
			if(register_a == 0x00)     { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			// TODO: Check for overflow{ BitSet(register_p, STATUS_BIT_OVERFLOW); } else { BitClear(register_p, STATUS_BIT_OVERFLOW); }
			if(register_a >= 0x80)     { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 2;
			cycles += 4; // + 1 for page
			break;
		case 0x7E:
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			result = Read((operand2 << 8) + operand1 + register_x);
			/* Check if carry flag is set to preserve value. */
			if(BitCheck(register_p, STATUS_BIT_CARRY)) {
				/* Yes, add 0x80 at end to preserve old carry flag. */
				/* Check if bit 0 is set to preserve value. */
				if(result & 0x01) {
					/* Yes, set carry flag to preserve bit 0. */
					BitSet(register_p, STATUS_BIT_CARRY);
				} else {
					/* No, clear carry flag to preserve bit 0. */
					BitClear(register_p, STATUS_BIT_CARRY);
				}
				/* Perform shift, add bit from carry flag. */
				result = (result >> 1) + 0x80;
			} else {
				/* No, add nothing at end to preserve old carry flag. */
				/* Check if bit 0 is set to preserve value. */
				if(result & 0x01) {
					/* Yes, set carry flag to preserve bit 0. */
					BitSet(register_p, STATUS_BIT_CARRY);
				} else {
					/* No, clear carry flag to preserve bit 0. */
					BitClear(register_p, STATUS_BIT_CARRY);
				}
				/* Perform shift, add nothing. */
				result = (result >> 1);
			}
			Write(((operand2 << 8) + operand1 + register_x), result);
			if(result == 0x00)         { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(result >= 0x80)         { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 2;
			cycles += 6;
			break;
		case 0x7F: IllegalOpcode(0x7F);
		case 0x80: IllegalOpcode(0x80);
		case 0x81:
			std::cout << "STA INDX unimp" << '\n';
			program_counter += 1;
			cycles += 6;
			break;
		case 0x82: IllegalOpcode(0x82);
		case 0x83: IllegalOpcode(0x83);
		case 0x84:
			operand1 = Read(program_counter + 1);
			Write(operand1, register_y);
			program_counter += 1;
			cycles += 3;
			break;
		case 0x85:
			operand1 = Read(program_counter + 1);
			Write(operand1, register_a);
			program_counter += 1;
			cycles += 3;
			break;
		case 0x86:
			operand1 = Read(program_counter + 1);
			Write(operand1, register_x);
			program_counter += 1;
			cycles += 3;
			break;
		case 0x87: IllegalOpcode(0x87);
		case 0x88:
			register_y--;
			if(register_y == 0x00)     {
				BitSet(register_p, STATUS_BIT_ZERO);
			}     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(register_y >= 0x80)     { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			cycles += 2;
			break;
		case 0x89: IllegalOpcode(0x89);
		case 0x8A:
			register_a = register_x;
			if(register_a == 0x00)     { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(register_a >= 0x80)     { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			cycles += 2;
			break;
		case 0x8B: IllegalOpcode(0x8B);
		case 0x8C:
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			Write(((operand2 << 8) + operand1), register_y);
			program_counter += 2;
			cycles += 4;
			break;
		case 0x8D:
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			Write(((operand2 << 8) + operand1), register_a);
			program_counter += 2;
			cycles += 4;
			break;
		case 0x8E:
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			Write(((operand2 << 8) + operand1), register_x);
			program_counter += 2;
			cycles += 4;
			break;
		case 0x8F: IllegalOpcode(0x8F);
		case 0x90:
			operand1 = Read(program_counter + 1);
			program_counter++;
			if(!BitCheck(register_p, STATUS_BIT_CARRY)) {
				// TODO: Add extra cycle if crossing page boundary (0x100).
				program_counter += (int8_t)operand1;
				cycles += 1;
			}
			cycles += 2;
			break;
		case 0x91:
			/* Read location from Zero-Page to load from. */
			operand1 = Read(program_counter + 1);
			/* Read the Zero-Page location plus one to get the upper half of the address. */
			operand2 = Read(operand1 + 1);
			/* Read the Zero-Page location to get the lower half of the address. */
			operand1 = Read(operand1);
			/* Finally, set the memory location to the contents of register A from operand 1 and 2 plus register Y. */
			Write(Read(((operand2 << 8) + operand1) + register_y), register_a);
			program_counter += 1;
			cycles += 6;
			break;
		case 0x92: IllegalOpcode(0x92);
		case 0x93: IllegalOpcode(0x93);
		case 0x94:
			/* Zero Page Y Index added here to force wrap around instead of carry in the line after. */
			operand1 = Read(program_counter + 1) + register_x;
			Write(operand1, register_y);
			program_counter += 1;
			cycles += 4;
			break;
		case 0x95:
			/* Zero Page X Index added here to force wrap around instead of carry in the line after. */
			operand1 = Read(program_counter + 1) + register_x;
			Write(operand1, register_a);
			program_counter += 1;
			cycles += 4;
			break;
		case 0x96:
			/* Zero Page Y Index added here to force wrap around instead of carry in the line after. */
			operand1 = Read(program_counter + 1) + register_y;
			Write(operand1, register_a);
			program_counter += 1;
			cycles += 4;
			break;
		case 0x97: IllegalOpcode(0x97);
		case 0x98:
			register_a = register_y;
			if(register_a == 0x00)      { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(register_a >= 0x80)      { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			cycles += 2;
			break;
		case 0x99:
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			Write(((operand2 << 8) + operand1 + register_y), register_a);
			program_counter += 2;
			cycles += 5;
			break;
		case 0x9A:
			register_s = register_x;
			cycles += 2;
			break;
		case 0x9B: IllegalOpcode(0x9B);
		case 0x9C: IllegalOpcode(0x9C);
		case 0x9D:
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			Write(((operand2 << 8) + operand1 + register_x), register_a);
			program_counter += 2;
			cycles += 5;
			break;
		case 0x9E: IllegalOpcode(0x9E);
		case 0x9F: IllegalOpcode(0x9F);
		case 0xA0:
			operand1 = Read(program_counter + 1);
			register_y = operand1;
			if(register_y == 0x00)      { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(register_y >= 0x80)      { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 1;
			cycles += 2;
			break;
		case 0xA1:
			std::cout << "LDA INDX unimp" << '\n';
			program_counter += 1;
			cycles += 6;
			break;
		case 0xA2:
			operand1 = Read(program_counter + 1);
			register_x = operand1;
			if(register_x == 0x00)      { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(register_x >= 0x80)      { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 1;
			cycles += 2;
			break;
		case 0xA3: IllegalOpcode(0xA3);
		case 0xA4:
			operand1 = Read(program_counter + 1);
			register_y = Read(operand1);
			if(register_y == 0x00)      { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(register_y >= 0x80)      { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 1;
			cycles += 3;
			break;
		case 0xA5:
			operand1 = Read(program_counter + 1);
			register_a = Read(operand1);
			if(register_a == 0x00)      { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(register_a >= 0x80)      { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 1;
			cycles += 3;
			break;
		case 0xA6:
			operand1 = Read(program_counter + 1);
			register_x = Read(operand1);
			if(register_x == 0x00)      { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(register_x >= 0x80)      { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 1;
			cycles += 3;
			break;
		case 0xA7: IllegalOpcode(0xA7);
		case 0xA8:
			register_y = register_a;
			if(register_y == 0x00)      { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(register_y >= 0x80)      { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			cycles += 2;
			break;
		case 0xA9:
			register_a = Read(program_counter + 1);
			if(register_a == 0x00)      { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(register_a >= 0x80)      { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 1;
			cycles += 2;
			break;
		case 0xAA:
			register_x = register_a;
			if(register_x == 0x00)      { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(register_x >= 0x80)      { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			cycles += 2;
			break;
		case 0xAB: IllegalOpcode(0xAB);
		case 0xAC:
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			register_y = Read((operand2 << 8) + operand1);
			if(register_y == 0x00)      { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(register_y >= 0x80)      { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 2;
			cycles += 4;
			break;
		case 0xAD:
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			register_a = Read((operand2 << 8) + operand1);
			if(register_a == 0x00)      { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(register_a >= 0x80)      { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 2;
			cycles += 4;
			break;
		case 0xAE:
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			register_x = Read((operand2 << 8) + operand1);
			if(register_x == 0x00)     { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(register_x >= 0x80)     { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 2;
			cycles += 4;
			break;
		case 0xAF: IllegalOpcode(0xAF);
		case 0xB0:
			operand1 = Read(program_counter + 1);
			program_counter++;
			if(BitCheck(register_p, STATUS_BIT_CARRY)) {
				// TODO: Add extra cycle if crossing page boundary (0x100).
				program_counter += (int8_t)operand1;
				cycles += 1;
			}
			cycles += 2;
			break;
		case 0xB1:
			/* Read location from Zero-Page to load from. */
			operand1 = Read(program_counter + 1);
			/* Read the Zero-Page location plus one to get the upper half of the address. */
			operand2 = Read(operand1 + 1);
			/* Read the Zero-Page location to get the lower half of the address. */
			operand1 = Read(operand1);
			/* Finally, set register A to be the contents of the memory location from operand 1 and 2 plus register Y. */
			register_a = Read(((operand2 << 8) + operand1) + register_y);
			if(register_a == 0x00)     { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(register_a >= 0x80)     { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 1;
			cycles += 5; // +1 for page
			break;
		case 0xB2: IllegalOpcode(0xB2);
		case 0xB3: IllegalOpcode(0xB3);
		case 0xB4:
			/* Zero Page X Index added here to force wrap around instead of carry in the line after. */
			operand1 = Read(program_counter + 1) + register_x;
			register_y = Read(operand1);
			if(register_y == 0x00)      { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(register_y >= 0x80)      { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 1;
			cycles += 4;
			break;
		case 0xB5:
			/* Zero Page X Index added here to force wrap around instead of carry in the line after. */
			operand1 = Read(program_counter + 1) + register_x;
			register_a = Read(operand1);
			if(register_a == 0x00)      { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(register_a >= 0x80)      { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 1;
			cycles += 4;
			break;
		case 0xB6:
			/* Zero Page Y Index added here to force wrap around instead of carry in the line after. */
			operand1 = Read(program_counter + 1) + register_y;
			register_x = Read(operand1);
			if(register_x == 0x00)      { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(register_x >= 0x80)      { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 1;
			cycles += 4;
			break;
		case 0xB7: IllegalOpcode(0xB7);
		case 0xB8:
			BitClear(register_p, STATUS_BIT_OVERFLOW);
			cycles += 2;
			break;
		case 0xB9:
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			register_a = Read((operand2 << 8) + operand1 + register_y);
			if(register_a == 0x00)     { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(register_a >= 0x80)     { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 2;
			cycles += 4; // +1 for page
			break;
		case 0xBA:
			register_x = register_s;
			if(register_x == 0x00)     { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(register_x >= 0x80)     { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			cycles += 2;
			break;
		case 0xBB: IllegalOpcode(0xBB);
		case 0xBC:
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			register_y = Read((operand2 << 8) + operand1 + register_x);
			if(register_y == 0x00)     { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(register_y >= 0x80)     { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 2;
			cycles += 4; // +1 for page
			break;
		case 0xBD:
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			register_a = Read((operand2 << 8) + operand1 + register_x);
			if(register_a == 0x00)     { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(register_a >= 0x80)     { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 2;
			cycles += 4; // +1 for page
			break;
		case 0xBE:
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			register_x = Read((operand2 << 8) + operand1 + register_y);
			if(register_x == 0x00)     { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(register_x >= 0x80)     { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 2;
			cycles += 4; // +1 for page
			break;
		case 0xBF: IllegalOpcode(0xBF);
		case 0xC0:
			operand1 = Read(program_counter + 1);
			result = (register_y - operand1);
			if(register_y >= operand1) { BitSet(register_p, STATUS_BIT_CARRY); }    else { BitClear(register_p, STATUS_BIT_CARRY); }
			if(register_y == operand1) { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(result     >= 0x80)     { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 1;
			cycles += 2;
			break;
		case 0xC1:
			std::cout << "CMP INDX unimp" << '\n';
			program_counter += 1;
			cycles += 6;
			break;
		case 0xC2: IllegalOpcode(0xC2);
		case 0xC3: IllegalOpcode(0xC3);
		case 0xC4:
			operand1 = Read(program_counter + 1);
			operand2 = Read(operand1);
			result = (register_y - operand2);
			if(register_y >= operand2) { BitSet(register_p, STATUS_BIT_CARRY); }    else { BitClear(register_p, STATUS_BIT_CARRY); }
			if(register_y == operand2) { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(result     >= 0x80)     { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 1;
			cycles += 3;
			break;
		case 0xC5:
			operand1 = Read(program_counter + 1);
			operand2 = Read(operand1);
			result = (register_a - operand2);
			if(register_a >= operand2) { BitSet(register_p, STATUS_BIT_CARRY); }    else { BitClear(register_p, STATUS_BIT_CARRY); }
			if(register_a == operand2) { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(result     >= 0x80)     { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 1;
			cycles += 3;
			break;
		case 0xC6:
			operand1 = Read(program_counter + 1);
			operand2 = Read(operand1);
			operand2--;
			Write(operand1, operand2);
			if(operand2 == 0x00)       { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(operand2 >= 0x80)       { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 1;
			cycles += 5;
			break;
		case 0xC7: IllegalOpcode(0xC7);
		case 0xC8:
			register_y++;
			if(register_y == 0x00)     { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(register_y >= 0x80)     { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			cycles += 2;
			break;
		case 0xC9:
			operand1 = Read(program_counter + 1);
			result = (register_a - operand1);
			if(register_a >= operand1) { BitSet(register_p, STATUS_BIT_CARRY); }    else { BitClear(register_p, STATUS_BIT_CARRY); }
			if(register_a == operand1) { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(result     >= 0x80)     { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 1;
			cycles += 2;
			break;
		case 0xCA:
			register_x--;
			if(register_x == 0x00)     { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(register_x >= 0x80)     { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			cycles += 2;
			break;
		case 0xCB: IllegalOpcode(0xCB);
		case 0xCC:
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			result = Read((operand2 << 8) + operand1);
			if(register_y >= result)   { BitSet(register_p, STATUS_BIT_CARRY); }    else { BitClear(register_p, STATUS_BIT_CARRY); }
			if(register_y == result)   { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			result = (register_y - result);
			if(result     >= 0x80)     { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 2;
			cycles += 4;
			break;
		case 0xCD:
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			result = Read((operand2 << 8) + operand1);
			if(register_a >= result)   { BitSet(register_p, STATUS_BIT_CARRY); }    else { BitClear(register_p, STATUS_BIT_CARRY); }
			if(register_a == result)   { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			result = (register_a - Read((operand2 << 8) + operand1));
			if(result     >= 0x80)     { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 2;
			cycles += 4;
			break;
		case 0xCE:
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			result = Read((operand2 << 8) + operand1);
			result--;
			Write((operand2 << 8) + operand1, result);
			if(result == 0x00)         { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(result >= 0x80)         { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 2;
			cycles += 6;
			break;
		case 0xCF: IllegalOpcode(0xCF);
		case 0xD0:
			operand1 = Read(program_counter + 1);
			program_counter++;
			if(BitCheck(register_p, STATUS_BIT_ZERO) == 0) {
				// TODO: Add extra cycle if crossing page boundary (0x100).
				program_counter += (int8_t)operand1;
				cycles += 1;
			}
			cycles += 2;
			break;
		case 0xD1:
			std::cout << "CMP INDY unimp" << '\n';
			program_counter += 1;
			cycles += 5; // +1 for page
			break;
		case 0xD2: IllegalOpcode(0xD2);
		case 0xD3: IllegalOpcode(0xD3);
		case 0xD4: IllegalOpcode(0xD4);
		case 0xD5:
			/* Zero Page X Index added here to force wrap around instead of carry in the line after. */
			operand1 = Read(program_counter + 1) + register_x;
			operand2 = Read(operand1);
			result = (register_a - operand2);
			if(register_a >= operand2) { BitSet(register_p, STATUS_BIT_CARRY); }    else { BitClear(register_p, STATUS_BIT_CARRY); }
			if(register_a == operand2) { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(result     >= 0x80)     { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 1;
			cycles += 4;
			break;
		case 0xD6:
			/* Zero Page X Index added here to force wrap around instead of carry in the line after. */
			operand1 = Read(program_counter + 1) + register_x;
			operand2 = Read(operand1);
			operand2--;
			Write(operand1, operand2);
			if(operand2 == 0x00)       { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(operand2 >= 0x80)       { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 1;
			cycles += 6;
			break;
		case 0xD7: IllegalOpcode(0xD7);
		case 0xD8:
			BitClear(register_p, STATUS_BIT_DECIMAL);
			cycles += 2;
			break;
		case 0xD9:
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			result = Read((operand2 << 8) + operand1 + register_y);
			if(register_a >= result)   { BitSet(register_p, STATUS_BIT_CARRY); }    else { BitClear(register_p, STATUS_BIT_CARRY); }
			if(register_a == result)   { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			result = (register_a - Read((operand2 << 8) + operand1 + register_y));
			if(result     >= 0x80)     { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 2;
			cycles += 4; // + 1 for page
			break;
		case 0xDA: IllegalOpcode(0xDA);
		case 0xDB: IllegalOpcode(0xDB);
		case 0xDC: IllegalOpcode(0xDC);
		case 0xDD:
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			result = Read((operand2 << 8) + operand1 + register_x);
			if(register_a >= result)   { BitSet(register_p, STATUS_BIT_CARRY); }    else { BitClear(register_p, STATUS_BIT_CARRY); }
			if(register_a == result)   { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			result = (register_a - Read((operand2 << 8) + operand1 + register_x));
			if(result     >= 0x80)     { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 2;
			cycles += 4; // + 1 for page
			break;
		case 0xDE:
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			result = Read((operand2 << 8) + operand1 + register_x);
			result--;
			Write(((operand2 << 8) + operand1 + register_x), result);
			if(result == 0x00)         { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(result >= 0x80)         { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 2;
			cycles += 7;
			break;
		case 0xDF: IllegalOpcode(0xDF);
		case 0xE0:
			operand1 = Read(program_counter + 1);
			result = (register_x - operand1);
			if(register_x >= operand1) { BitSet(register_p, STATUS_BIT_CARRY); }    else { BitClear(register_p, STATUS_BIT_CARRY); }
			if(register_x == operand1) { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(result     >= 0x80)     { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 1;
			cycles += 2;
			break;
		case 0xE1:
			std::cout << "SBC INDX unimp" << '\n';
			program_counter += 2;
			cycles += 6;
			break;
		case 0xE2: IllegalOpcode(0xE2);
		case 0xE3: IllegalOpcode(0xE3);
		case 0xE4:
			operand1 = Read(program_counter + 1);
			operand1 = Read(operand1);
			result = (register_x - operand1);
			if(register_x >= operand1) { BitSet(register_p, STATUS_BIT_CARRY); }    else { BitClear(register_p, STATUS_BIT_CARRY); }
			if(register_x == operand1) { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(result     >= 0x80)     { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 1;
			cycles += 3;
			break;
		case 0xE5:
			operand1 = Read(program_counter + 1);
			operand1 = Read(operand1);
			register_a -= (operand1 + !BitCheck(register_p, STATUS_BIT_CARRY)); // TODO: Overflow flag needs to be set.
			if(register_a == 0x00)     { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(register_a >= 0x80)     { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 1;
			cycles += 3;
			break;
		case 0xE6:
			operand1 = Read(program_counter + 1);
			operand2 = Read(operand1);
			operand2++;
			Write(operand1, operand2);
			if(result == 0x00)         { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(result >= 0x80)         { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 1;
			cycles += 5;
			break;
		case 0xE7: IllegalOpcode(0xE7);
		case 0xE8:
			register_x++;
			if(register_x == 0x00)     { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(register_x >= 0x80)     { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			cycles += 2;
			break;
		case 0xE9:
			operand1 = Read(program_counter + 1);
			register_a -= (operand1 + !BitCheck(register_p, STATUS_BIT_CARRY)); // TODO: Overflow flag needs to be set.
			if(register_a == 0x00)     { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(register_a >= 0x80)     { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 1;
			cycles += 2;
			break;
		case 0xEA:
			cycles += 2;
			break;
		case 0xEB: IllegalOpcode(0xEB);
		case 0xEC:
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			result = Read((operand2 << 8) + operand1);
			if(register_x >= result)   { BitSet(register_p, STATUS_BIT_CARRY); }    else { BitClear(register_p, STATUS_BIT_CARRY); }
			if(register_x == result)   { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			result = (register_x - result);
			if(result     >= 0x80)     { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 2;
			cycles += 4;
			break;
		case 0xED:
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			result = Read((operand2 << 8) + operand1);
			register_a -= (result + !BitCheck(register_p, STATUS_BIT_CARRY)); // TODO: Overflow flag needs to be set.
			if(register_a == 0x00)     { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(register_a >= 0x80)     { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 2;
			cycles += 4;
			break;
		case 0xEE:
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			result = Read((operand2 << 8) + operand1);
			result++;
			Write((operand2 << 8) + operand1, result);
			if(result == 0x00)         { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(result >= 0x80)         { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 2;
			cycles += 6;
			break;
		case 0xEF: IllegalOpcode(0xEF);
		case 0xF0:
			operand1 = Read(program_counter + 1);
			program_counter++;
			if(BitCheck(register_p, STATUS_BIT_ZERO) == 1) {
				// TODO: Add extra cycle if crossing page boundary (0x100).
				program_counter += (int8_t)operand1;
				cycles += 1;
			}
			cycles += 2;
			break;
		case 0xF1:
			std::cout << "SBC INDY unimp" << '\n';
			program_counter += 1;
			cycles += 5; // +1 page
			break;
		case 0xF2: IllegalOpcode(0xF2);
		case 0xF3: IllegalOpcode(0xF3);
		case 0xF4: IllegalOpcode(0xF4);
		case 0xF5:
			/* Zero Page X Index added here to force wrap around instead of carry in the line after. */
			operand1 = Read(program_counter + 1) + register_x;
			operand1 = Read(operand1);
			register_a -= (operand1 + !BitCheck(register_p, STATUS_BIT_CARRY)); // TODO: Overflow flag needs to be set.
			if(register_a == 0x00)     { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(register_a >= 0x80)     { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 1;
			cycles += 4;
			break;
		case 0xF6:
			/* Zero Page X Index added here to force wrap around instead of carry in the line after. */
			operand1 = Read(program_counter + 1) + register_x;
			operand2 = Read(operand1);
			operand2++;
			Write(operand1, operand2);
			if(result == 0x00)         { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(result >= 0x80)         { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 1;
			cycles += 6;
			break;
		case 0xF7: IllegalOpcode(0xF7);
		case 0xF8:
			BitSet(register_p, STATUS_BIT_DECIMAL);
			cycles += 2;
			break;
		case 0xF9:
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			result = Read((operand2 << 8) + operand1 + register_y);
			register_a -= (result + !BitCheck(register_p, STATUS_BIT_CARRY)); // TODO: Overflow flag needs to be set.
			if(register_a == 0x00)     { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(register_a >= 0x80)     { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 2;
			cycles += 4; // + 1 for page
			break;
		case 0xFA: IllegalOpcode(0xFA);
		case 0xFB: IllegalOpcode(0xFB);
		case 0xFC: IllegalOpcode(0xFC);
		case 0xFD:
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			result = Read((operand2 << 8) + operand1 + register_x);
			register_a -= (result + !BitCheck(register_p, STATUS_BIT_CARRY)); // TODO: Overflow flag needs to be set.
			if(register_a == 0x00)     { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(register_a >= 0x80)     { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 2;
			cycles += 4; // + 1 for page
			break;
		case 0xFE:
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			result = Read((operand2 << 8) + operand1 + register_x);
			result++;
			Write(((operand2 << 8) + operand1 + register_x), result);
			if(result == 0x00)         { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(result >= 0x80)         { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 2;
			cycles += 7;
			break;
		case 0xFF: IllegalOpcode(0xFF);
		default:
			std::cout << "Reached normally unreachable default in switch statement." << '\n';
			while (1);
			break;
	}

	if(increment_pc) {
		program_counter++;
	} else {
		increment_pc = true;
	}
}