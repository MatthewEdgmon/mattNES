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

	if(halt_on_illegal_opcode && illegal_opcode_triggered) {
		std::cout << "Caught Illegal Opcode " << HEX2(opcode) << " at " << HEX4(program_counter) << '\n';
		nes_system->DumpTestInfo();

		std::cout << "Halting CPU...\n";
		halted = true;
		while(1) {
			SDL_Delay(1);
		}
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
			Interrupt(IRQ_BRK);
			break;
		case 0x01:
			operand1 = Read(program_counter + 1);
			operand1 = Read(operand1 + register_x);
			register_a |= operand1;
			if(register_a == 0x00)     { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
            if(register_a >= 0x80)     { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 1;
			break;
		case 0x02: IllegalOpcode(0x02); break;
		case 0x03: IllegalOpcode(0x03); break;
		case 0x04: IllegalOpcode(0x04); break;
		case 0x05:
			operand1 = Read(program_counter + 1);
			operand1 = Read(operand1);
			register_a |= operand1;
			if(register_a == 0x00)     { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
            if(register_a >= 0x80)     { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 1;
			break;
		case 0x06:
			operand1 = Read(program_counter + 1);
			operand2 = Read(operand1);
			/* Check if bit 7 is set to preserve value. */
			if(operand2 &  0x80)       { BitSet(register_p, STATUS_BIT_CARRY); }    else { BitClear(register_p, STATUS_BIT_CARRY); }
			operand2 <<= 1;
			Write(operand1, operand2);
			if(operand2 == 0x00)       { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(operand2 >= 0x80)       { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 1;
			break;
		case 0x07:
			illegal_opcode_triggered = true;
			/* ASL */
			operand1 = Read(program_counter + 1);
			operand2 = Read(operand1);
			if(operand2 &  0x80)       { BitSet(register_p, STATUS_BIT_CARRY); }    else { BitClear(register_p, STATUS_BIT_CARRY); }
			operand2 <<= 1;
			Write(operand1, operand2);
			if(operand2 == 0x00)       { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(operand2 >= 0x80)       { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			/* ORA */
			operand1 = Read(program_counter + 1);
			operand1 = Read(operand1);
			register_a |= operand1;
			if(register_a == 0x00)     { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
            if(register_a >= 0x80)     { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 1;
			break;
		case 0x08:
			Push(register_p);
			break;
		case 0x09:
			operand1 = Read(program_counter + 1);
			register_a |= operand1;
            if(register_a == 0x00)     { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
            if(register_a >= 0x80)     { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 1;
			break;
		case 0x0A:
			/* Check if bit 7 is set to preserve value. */
			if(register_a &  0x80)     { BitSet(register_p, STATUS_BIT_CARRY); }    else { BitClear(register_p, STATUS_BIT_CARRY); }
			register_a <<= 1;
			if(register_a == 0x00)     { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(register_a >= 0x80)     { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			break;
		case 0x0B: IllegalOpcode(0x0B); break;
		case 0x0C: IllegalOpcode(0x0C); break;
		case 0x0D:
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			register_a |= Read((operand2 << 8) + operand1);
            if(register_a == 0x00)     { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
            if(register_a >= 0x80)     { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 2;
			break;
		case 0x0E:
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			result = Read((operand2 << 8) + operand1);
			/* Check if bit 7 is set to preserve value. */
			if(result &  0x80)     { BitSet(register_p, STATUS_BIT_CARRY); }    else { BitClear(register_p, STATUS_BIT_CARRY); }
			result <<= 1;
			Write((operand2 << 8) + operand1, result);
			if(result == 0x00)     { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(result >= 0x80)     { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 2;
			break;
		case 0x0F: IllegalOpcode(0x0F); break;
		case 0x10:
			operand1 = Read(program_counter + 1);
			program_counter++;
			if(BitCheck(register_p, STATUS_BIT_NEGATIVE) == 0) {
				// TODO: Add extra cycle if crossing page boundary (0x100).
				program_counter += (int8_t)operand1;
				cycles += 1;
			}
			break;
		case 0x11:
			std::cout << "ORA INDY unimp" << '\n';
			program_counter += 1;
			break;
		case 0x12: IllegalOpcode(0x12); break;
		case 0x13: IllegalOpcode(0x13); break;
		case 0x14: IllegalOpcode(0x14); break;
		case 0x15:
			/* Zero Page X Index added here to force wrap around instead of carry in the line after. */
			operand1 = (Read(program_counter + 1) + register_x);
			operand1 = Read(operand1);
			register_a |= operand1;
            if(register_a == 0x00)     { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
            if(register_a >= 0x80)     { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 1;
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
			break;
		case 0x17: IllegalOpcode(0x17); break;
		case 0x18:
			BitClear(register_p, STATUS_BIT_CARRY);
			break;
		case 0x19:
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			register_a |= Read((operand2 << 8) + operand1 + register_y);
            if(register_a == 0x00)     { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
            if(register_a >= 0x80)     { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 1;
			// TODO: Add 1 to cycle count if crossing page boundary.
			break;
		case 0x1A: IllegalOpcode(0x1A); break;
		case 0x1B: IllegalOpcode(0x1B); break;
		case 0x1C: IllegalOpcode(0x1C); break;
		case 0x1D:
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			register_a |= Read((operand2 << 8) + operand1 + register_x);
            if(register_a == 0x00)     { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
            if(register_a >= 0x80)     { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 1;
			// TODO: Add 1 to cycle count if crossing page boundary.
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
			break;
		case 0x1F: IllegalOpcode(0x1F); break;
		case 0x20:
			/* Set the return address three bytes ahead of current PC. TODO: Implement accurate FDE to make PC only plus two. */
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			program_counter += 3; 
			Push(program_counter >> 8);
			Push(program_counter);
			program_counter = (operand2 << 8) + operand1;
			increment_pc = false;
			break;
		case 0x21:
			operand1 = Read(program_counter + 1);
			operand1 = Read(operand1 + register_x); // TODO: Not done right.
			register_a &= operand1;
			if(register_a == 0x00)     { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(register_a >= 0x80)     { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 1;
			break;
		case 0x22: IllegalOpcode(0x22); break;
		case 0x23: IllegalOpcode(0x23); break;
		case 0x24:
			operand1 = Read(program_counter + 1);
			operand1 = Read(operand1);
			if((operand1 & register_a) == 0) { BitSet(register_p, STATUS_BIT_ZERO); } else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(BitCheck(operand1, 6))  { BitSet(register_p, STATUS_BIT_OVERFLOW); } else { BitClear(register_p, STATUS_BIT_OVERFLOW); }
			if(BitCheck(operand1, 7))  { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 1;
			break;
		case 0x25:
			operand1 = Read(program_counter + 1);
			operand1 = Read(operand1);
			register_a &= operand1;
			if(register_a == 0x00)     { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(register_a >= 0x80)     { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 1;
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
			break;
		case 0x27: IllegalOpcode(0x27); break;
		case 0x28:
			register_p = Pop();
			break;
		case 0x29:
			operand1 = Read(program_counter + 1);
			register_a &= operand1;
			if(register_a == 0x00)     { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(register_a >= 0x80)     { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 1;
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
			break;
		case 0x2B: IllegalOpcode(0x2B); break;
		case 0x2C:
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			operand1 = Read((operand2 << 8) + operand1);
			if((operand1 & register_a) == 0) { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(BitCheck(operand1, 6))        { BitSet(register_p, STATUS_BIT_OVERFLOW); } else { BitClear(register_p, STATUS_BIT_OVERFLOW); }
			if(BitCheck(operand1, 7))        { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 2;
			break;
		case 0x2D:
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			register_a &= Read((operand2 << 8) + operand1);
			if(register_a == 0x00)     { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(register_a >= 0x80)     { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 2;
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
			break;
		case 0x2F: IllegalOpcode(0x2F); break;
		case 0x30:
			operand1 = Read(program_counter + 1);
			program_counter++;
			if(BitCheck(register_p, STATUS_BIT_NEGATIVE)) {
				program_counter += (int8_t)operand1;
				cycles += 1;
				// TODO: Add 1 to cycle count if crossing page boundary.
			}
			break;
		case 0x31:
			std::cout << "AND INDY unimp" << '\n';
			program_counter += 1;
			break;
		case 0x32: IllegalOpcode(0x32); break;
		case 0x33: IllegalOpcode(0x33); break;
		case 0x34: IllegalOpcode(0x34); break;
		case 0x35:
			/* Zero Page X Index added here to force wrap around instead of carry in the line after. */
			operand1 = Read(program_counter + 1) + register_x;
			operand2 = Read(operand1);
			register_a &= operand2;
			if(register_a == 0x00)     { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(register_a >= 0x80)     { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 1;
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
			break;
		case 0x37: IllegalOpcode(0x37); break;
		case 0x38:
			BitSet(register_p, STATUS_BIT_CARRY);
			break;
		case 0x39:
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			register_a &= Read((operand2 << 8) + operand1 + register_y);
			if(register_a == 0x00)     { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(register_a >= 0x80)     { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 2;
			// TODO: Add 1 to cycle count if crossing page boundary.
			break;
		case 0x3A: IllegalOpcode(0x3A); break;
		case 0x3B: IllegalOpcode(0x3B); break;
		case 0x3C: IllegalOpcode(0x3C); break;
		case 0x3D:
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			register_a &= Read((operand2 << 8) + operand1 + register_x);
			if(register_a == 0x00)     { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(register_a >= 0x80)     { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 2;
			// TODO: Add 1 to cycle count if crossing page boundary.
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
			break;
		case 0x3F: IllegalOpcode(0x3F); break;
		case 0x40:
			/* Pop flags, high byte, and low byte in that order. */
			register_p = Pop();
			program_counter  = Pop() << 8;
			program_counter += Pop();
			increment_pc = false;
			break;
		case 0x41:
			operand1 = Read(program_counter + 1);
			operand1 = Read(operand1 + register_x);
			register_a ^= operand1;
			if(register_a == 0x00)     { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(register_a >= 0x80)     { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 1;
			break;
		case 0x42: IllegalOpcode(0x42); break;
		case 0x43: IllegalOpcode(0x43); break;
		case 0x44: IllegalOpcode(0x44); break;
		case 0x45:
			operand1 = Read(program_counter + 1);
			operand1 = Read(operand1);
			register_a ^= operand1;
			if(register_a == 0x00)     { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(register_a >= 0x80)     { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 1;
			break;
		case 0x46:
			operand1 = Read(program_counter + 1);
			operand2 = Read(operand1);
			/* Check if bit 0 is set to preserve value. */
			if(operand2 &  0x01)       { BitSet(register_p, STATUS_BIT_CARRY); }    else { BitClear(register_p, STATUS_BIT_CARRY); }
			operand2 >>= 1;
			Write(operand1, operand2);
			if(operand2 == 0x00)       { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(operand2 >= 0x80)       { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 1;
			break;
		case 0x47: IllegalOpcode(0x47); break;
		case 0x48:
			Push(register_a);
			break;
		case 0x49:
			operand1 = Read(program_counter + 1);
			register_a ^= operand1;
			if(register_a == 0x00)     { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(register_a >= 0x80)     { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 1;
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
			break;
		case 0x4B: IllegalOpcode(0x4B); break;
		case 0x4C:
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			program_counter = (operand2 << 8) + operand1;
			increment_pc = false;
			break;
		case 0x4D:
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			register_a ^= Read((operand2 << 8) + operand1);
			if(register_a == 0x00)     { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(register_a >= 0x80)     { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 2;
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
			break;
		case 0x4F: IllegalOpcode(0x4F); break;
		case 0x50:
			operand1 = Read(program_counter + 1);
			program_counter++;
			if(!BitCheck(register_p, STATUS_BIT_OVERFLOW)) {
				program_counter += (int8_t)operand1;
				cycles += 1;
				// TODO: Add 1 to cycle count if crossing page boundary.
			}
			break;
		case 0x51:
			std::cout << "EOR INDY unimp" << '\n';
			program_counter += 1;
			// TODO: Add 1 to cycle count if crossing page boundary.
			break;
		case 0x52: IllegalOpcode(0x52); break;
		case 0x53: IllegalOpcode(0x53); break;
		case 0x54: IllegalOpcode(0x54); break;
		case 0x55:
			/* Zero Page X Index added here to force wrap around here instead of carry in the line after. */
			operand1 = Read(program_counter + 1) + register_x;
			operand1 = Read(operand1);
			register_a ^= operand1;
			if(register_a == 0x00)     { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(register_a >= 0x80)     { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 1;
			break;
		case 0x56:
			/* Zero Page X Index added here to force wrap around here instead of carry in the line after. */
			operand1 = Read(program_counter + 1) + register_x;
			operand2 = Read(operand1);
			/* Check if bit 0 is set to preserve value. */
			if(operand2 &  0x01)       { BitSet(register_p, STATUS_BIT_CARRY); }    else { BitClear(register_p, STATUS_BIT_CARRY); }
			operand2 >>= 1;
			Write(operand1, operand2);
			if(operand2 == 0x00)       { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(operand2 >= 0x80)       { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 1;
			break;
		case 0x57: IllegalOpcode(0x57); break;
		case 0x58:
			BitClear(register_p, STATUS_BIT_INTERRUPT_DISABLE);
			break;
		case 0x59:
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			register_a ^= Read((operand2 << 8) + operand1 + register_y);
			if(register_a == 0x00)     { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(register_a >= 0x80)     { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 2;
			// TODO: Add 1 to cycle count if crossing page boundary.
			break;
		case 0x5A: IllegalOpcode(0x5A); break;
		case 0x5B: IllegalOpcode(0x5B); break;
		case 0x5C: IllegalOpcode(0x5C); break;
		case 0x5D:
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			register_a ^= Read((operand2 << 8) + operand1 + register_x);
			if(register_a == 0x00)     { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(register_a >= 0x80)     { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 2;
			// TODO: Add 1 to cycle count if crossing page boundary.
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
			break;
		case 0x5F: IllegalOpcode(0x5F); break;
		case 0x60:
			program_counter =  Pop();
			program_counter |= Pop() << 8;
			increment_pc = false;
			break;
		case 0x61:
			/* Zero Page X Index added here to force wrap around here instead of carry in the line after. */
			operand1 = Read(program_counter + 1) + register_x;
			operand1 = Read(operand1);
			result = register_a + operand1 + BitCheck(register_p, STATUS_BIT_CARRY);
			/* Check carry/unsigned overflow. */
			if(result &  0x100)        { BitSet(register_p, STATUS_BIT_CARRY); }    else { BitClear(register_p, STATUS_BIT_CARRY); }
			/* Check signed overflow. */
			if((register_a ^ result) & (operand1 ^ result) & 0x80)
									   { BitSet(register_p, STATUS_BIT_OVERFLOW); } else { BitClear(register_p, STATUS_BIT_OVERFLOW); }
			register_a = (uint8_t) result; 
			if(register_a == 0x00)     { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(register_a >= 0x80)     { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 1;
			break;
		case 0x62: IllegalOpcode(0x62); break;
		case 0x63: IllegalOpcode(0x63); break;
		case 0x64: IllegalOpcode(0x64); break;
		case 0x65:
			operand1 = Read(program_counter + 1);
			operand1 = Read(operand1);
			result = register_a + operand1 + BitCheck(register_p, STATUS_BIT_CARRY);
			/* Check carry/unsigned overflow. */
			if(result &  0x100)        { BitSet(register_p, STATUS_BIT_CARRY); }    else { BitClear(register_p, STATUS_BIT_CARRY); }
			/* Check signed overflow. */
			if((register_a ^ result) & (operand1 ^ result) & 0x80)
									   { BitSet(register_p, STATUS_BIT_OVERFLOW); } else { BitClear(register_p, STATUS_BIT_OVERFLOW); }
			register_a = (uint8_t) result; 
			if(register_a == 0x00)     { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(register_a >= 0x80)     { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 1;
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
			break;
		case 0x67: IllegalOpcode(0x67); break;
		case 0x68:
			register_a = Pop();
			if(register_a == 0x00)     { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(register_a >= 0x80)     { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			break;
		case 0x69:
			operand1 = Read(program_counter + 1);
			result = register_a + operand1 + BitCheck(register_p, STATUS_BIT_CARRY);
			/* Check carry/unsigned overflow. */
			if(result &  0x100)        { BitSet(register_p, STATUS_BIT_CARRY); }    else { BitClear(register_p, STATUS_BIT_CARRY); }
			/* Check signed overflow. */
			if((register_a ^ result) & (operand1 ^ result) & 0x80)
									   { BitSet(register_p, STATUS_BIT_OVERFLOW); } else { BitClear(register_p, STATUS_BIT_OVERFLOW); }
			register_a = (uint8_t) result; 
			if(register_a == 0x00)     { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(register_a >= 0x80)     { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 1;
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
			break;
		case 0x6B: IllegalOpcode(0x6B); break;
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
			break;
		case 0x6D:
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			result = register_a + Read((operand2 << 8) + operand1) + BitCheck(register_p, STATUS_BIT_CARRY);
			/* Check carry/unsigned overflow. */
			if(result &  0x100)        { BitSet(register_p, STATUS_BIT_CARRY); }    else { BitClear(register_p, STATUS_BIT_CARRY); }
			/* Check signed overflow. */
			if((register_a ^ result) & (Read((operand2 << 8) + operand1) ^ result) & 0x80)
									   { BitSet(register_p, STATUS_BIT_OVERFLOW); } else { BitClear(register_p, STATUS_BIT_OVERFLOW); }
			register_a = (uint8_t) result; 
			if(register_a == 0x00)     { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(register_a >= 0x80)     { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 2;
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
			break;
		case 0x6F: IllegalOpcode(0x6F); break;
		case 0x70:
			operand1 = Read(program_counter + 1);
			program_counter++;
			if(BitCheck(register_p, STATUS_BIT_OVERFLOW)) {
				program_counter += (int8_t)operand1;
				cycles += 1;
				// TODO: Add 1 to cycle count if crossing page boundary.
			}
			break;
		case 0x71:
			std::cout << "ADC INDY unimp" << '\n';
			program_counter += 1;
			// TODO: Add 1 to cycle count if crossing page boundary.
			break;
		case 0x72: IllegalOpcode(0x72); break;
		case 0x73: IllegalOpcode(0x73); break;
		case 0x74: IllegalOpcode(0x74); break;
		case 0x75:
			/* Zero Page X Index added here to force wrap around instead of carry in the line after. */
			operand1 = Read(program_counter + 1) + register_x;
			operand1 = Read(operand1);
			result = register_a + operand1 + BitCheck(register_p, STATUS_BIT_CARRY);
			/* Check carry/unsigned overflow. */
			if(result &  0x100)        { BitSet(register_p, STATUS_BIT_CARRY); }    else { BitClear(register_p, STATUS_BIT_CARRY); }
			/* Check signed overflow. */
			if((register_a ^ result) & (operand1 ^ result) & 0x80)
									   { BitSet(register_p, STATUS_BIT_OVERFLOW); } else { BitClear(register_p, STATUS_BIT_OVERFLOW); }
			register_a = (uint8_t) result; 
			if(register_a == 0x00)     { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(register_a >= 0x80)     { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 1;
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
			break;
		case 0x77: IllegalOpcode(0x77); break;
		case 0x78:
			BitSet(register_p, STATUS_BIT_INTERRUPT_DISABLE);
			break;
		case 0x79:
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			result = register_a + Read((operand2 << 8) + operand1 + register_y) + BitCheck(register_p, STATUS_BIT_CARRY);
			/* Check carry/unsigned overflow. */
			if(result &  0x100)        { BitSet(register_p, STATUS_BIT_CARRY); }    else { BitClear(register_p, STATUS_BIT_CARRY); }
			/* Check signed overflow. */
			if((register_a ^ result) & (Read((operand2 << 8) + operand1 + register_y) ^ result) & 0x80)
									   { BitSet(register_p, STATUS_BIT_OVERFLOW); } else { BitClear(register_p, STATUS_BIT_OVERFLOW); }
			register_a = (uint8_t) result; 
			if(register_a == 0x00)     { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(register_a >= 0x80)     { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 2;
			// TODO: Add 1 to cycle count if crossing page boundary.
			break;
		case 0x7A: IllegalOpcode(0x7A); break;
		case 0x7B: IllegalOpcode(0x7B); break;
		case 0x7C: IllegalOpcode(0x7C); break;
		case 0x7D:
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			result = register_a + Read((operand2 << 8) + operand1 + register_x) + BitCheck(register_p, STATUS_BIT_CARRY);
			/* Check carry/unsigned overflow. */
			if(result &  0x100)        { BitSet(register_p, STATUS_BIT_CARRY); }    else { BitClear(register_p, STATUS_BIT_CARRY); }
			/* Check signed overflow. */
			if((register_a ^ result) & (Read((operand2 << 8) + operand1 + register_x) ^ result) & 0x80)
									   { BitSet(register_p, STATUS_BIT_OVERFLOW); } else { BitClear(register_p, STATUS_BIT_OVERFLOW); }
			register_a = (uint8_t) result; 
			if(register_a == 0x00)     { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(register_a >= 0x80)     { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 2;
			// TODO: Add 1 to cycle count if crossing page boundary.
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
			break;
		case 0x7F: IllegalOpcode(0x7F); break;
		case 0x80: IllegalOpcode(0x80); break;
		case 0x81:
			operand1 = Read(program_counter + 1);
			operand1 = Read(operand1 + register_x);
			Write(operand1, register_a);
			program_counter += 1;
			break;
		case 0x82: IllegalOpcode(0x82); break;
		case 0x83: IllegalOpcode(0x83); break;
		case 0x84:
			operand1 = Read(program_counter + 1);
			Write(operand1, register_y);
			program_counter += 1;
			break;
		case 0x85:
			operand1 = Read(program_counter + 1);
			Write(operand1, register_a);
			program_counter += 1;
			break;
		case 0x86:
			operand1 = Read(program_counter + 1);
			Write(operand1, register_x);
			program_counter += 1;
			break;
		case 0x87: IllegalOpcode(0x87); break;
		case 0x88:
			register_y--;
			if(register_y == 0x00)     { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(register_y >= 0x80)     { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			break;
		case 0x89: IllegalOpcode(0x89); break;
		case 0x8A:
			register_a = register_x;
			if(register_a == 0x00)     { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(register_a >= 0x80)     { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			break;
		case 0x8B: IllegalOpcode(0x8B); break;
		case 0x8C:
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			Write(((operand2 << 8) + operand1), register_y);
			program_counter += 2;
			break;
		case 0x8D:
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			Write(((operand2 << 8) + operand1), register_a);
			program_counter += 2;
			break;
		case 0x8E:
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			Write(((operand2 << 8) + operand1), register_x);
			program_counter += 2;
			break;
		case 0x8F: IllegalOpcode(0x8F); break;
		case 0x90:
			operand1 = Read(program_counter + 1);
			program_counter++;
			if(!BitCheck(register_p, STATUS_BIT_CARRY)) {
				program_counter += (int8_t)operand1;
				cycles += 1;
				// TODO: Add 1 to cycle count if crossing page boundary.
			}
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
			break;
		case 0x92: IllegalOpcode(0x92); break;
		case 0x93: IllegalOpcode(0x93); break;
		case 0x94:
			/* Zero Page Y Index added here to force wrap around instead of carry in the line after. */
			operand1 = Read(program_counter + 1) + register_x;
			Write(operand1, register_y);
			program_counter += 1;
			break;
		case 0x95:
			/* Zero Page X Index added here to force wrap around instead of carry in the line after. */
			operand1 = Read(program_counter + 1) + register_x;
			Write(operand1, register_a);
			program_counter += 1;
			break;
		case 0x96:
			/* Zero Page Y Index added here to force wrap around instead of carry in the line after. */
			operand1 = Read(program_counter + 1) + register_y;
			Write(operand1, register_a);
			program_counter += 1;
			break;
		case 0x97: IllegalOpcode(0x97); break;
		case 0x98:
			register_a = register_y;
			if(register_a == 0x00)      { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(register_a >= 0x80)      { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			break;
		case 0x99:
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			Write(((operand2 << 8) + operand1 + register_y), register_a);
			program_counter += 2;
			break;
		case 0x9A:
			register_s = register_x;
			break;
		case 0x9B: IllegalOpcode(0x9B); break;
		case 0x9C: IllegalOpcode(0x9C); break;
		case 0x9D:
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			Write(((operand2 << 8) + operand1 + register_x), register_a);
			program_counter += 2;
			break;
		case 0x9E: IllegalOpcode(0x9E); break;
		case 0x9F: IllegalOpcode(0x9F); break;
		case 0xA0:
			operand1 = Read(program_counter + 1);
			register_y = operand1;
			if(register_y == 0x00)      { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(register_y >= 0x80)      { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 1;
			break;
		case 0xA1:
			/* Zero Page X Index added here to force wrap around instead of carry in the line after. */
			operand1 = Read(program_counter + 1) + register_x;
			register_a = operand1;
			if(register_a == 0x00)      { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(register_a >= 0x80)      { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 1;
			break;
		case 0xA2:
			operand1 = Read(program_counter + 1);
			register_x = operand1;
			if(register_x == 0x00)      { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(register_x >= 0x80)      { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 1;
			break;
		case 0xA3: IllegalOpcode(0xA3); break;
		case 0xA4:
			operand1 = Read(program_counter + 1);
			register_y = Read(operand1);
			if(register_y == 0x00)      { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(register_y >= 0x80)      { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 1;
			break;
		case 0xA5:
			operand1 = Read(program_counter + 1);
			register_a = Read(operand1);
			if(register_a == 0x00)      { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(register_a >= 0x80)      { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 1;
			break;
		case 0xA6:
			operand1 = Read(program_counter + 1);
			register_x = Read(operand1);
			if(register_x == 0x00)      { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(register_x >= 0x80)      { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 1;
			break;
		case 0xA7: IllegalOpcode(0xA7); break;
		case 0xA8:
			register_y = register_a;
			if(register_y == 0x00)      { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(register_y >= 0x80)      { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			break;
		case 0xA9:
			register_a = Read(program_counter + 1);
			if(register_a == 0x00)      { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(register_a >= 0x80)      { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 1;
			break;
		case 0xAA:
			register_x = register_a;
			if(register_x == 0x00)      { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(register_x >= 0x80)      { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			break;
		case 0xAB: IllegalOpcode(0xAB); break;
		case 0xAC:
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			register_y = Read((operand2 << 8) + operand1);
			if(register_y == 0x00)      { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(register_y >= 0x80)      { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 2;
			break;
		case 0xAD:
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			register_a = Read((operand2 << 8) + operand1);
			if(register_a == 0x00)      { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(register_a >= 0x80)      { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 2;
			break;
		case 0xAE:
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			register_x = Read((operand2 << 8) + operand1);
			if(register_x == 0x00)     { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(register_x >= 0x80)     { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 2;
			break;
		case 0xAF: IllegalOpcode(0xAF); break;
		case 0xB0:
			operand1 = Read(program_counter + 1);
			program_counter++;
			if(BitCheck(register_p, STATUS_BIT_CARRY)) {
				program_counter += (int8_t)operand1;
				cycles += 1;
				// TODO: Add 1 to cycle count if crossing page boundary.
			}
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
			// TODO: Add 1 to cycle count if crossing page boundary.
			break;
		case 0xB2: IllegalOpcode(0xB2); break;
		case 0xB3: IllegalOpcode(0xB3); break;
		case 0xB4:
			/* Zero Page X Index added here to force wrap around instead of carry in the line after. */
			operand1 = Read(program_counter + 1) + register_x;
			register_y = Read(operand1);
			if(register_y == 0x00)      { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(register_y >= 0x80)      { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 1;
			break;
		case 0xB5:
			/* Zero Page X Index added here to force wrap around instead of carry in the line after. */
			operand1 = Read(program_counter + 1) + register_x;
			register_a = Read(operand1);
			if(register_a == 0x00)      { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(register_a >= 0x80)      { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 1;
			break;
		case 0xB6:
			/* Zero Page Y Index added here to force wrap around instead of carry in the line after. */
			operand1 = Read(program_counter + 1) + register_y;
			register_x = Read(operand1);
			if(register_x == 0x00)      { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(register_x >= 0x80)      { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 1;
			break;
		case 0xB7: IllegalOpcode(0xB7); break;
		case 0xB8:
			BitClear(register_p, STATUS_BIT_OVERFLOW);
			break;
		case 0xB9:
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			register_a = Read((operand2 << 8) + operand1 + register_y);
			if(register_a == 0x00)     { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(register_a >= 0x80)     { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 2;
			// TODO: Add 1 to cycle count if crossing page boundary.
			break;
		case 0xBA:
			register_x = register_s;
			if(register_x == 0x00)     { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(register_x >= 0x80)     { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			break;
		case 0xBB: IllegalOpcode(0xBB); break;
		case 0xBC:
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			register_y = Read((operand2 << 8) + operand1 + register_x);
			if(register_y == 0x00)     { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(register_y >= 0x80)     { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 2;
			// TODO: Add 1 to cycle count if crossing page boundary.
			break;
		case 0xBD:
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			register_a = Read((operand2 << 8) + operand1 + register_x);
			if(register_a == 0x00)     { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(register_a >= 0x80)     { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 2;
			// TODO: Add 1 to cycle count if crossing page boundary.
			break;
		case 0xBE:
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			register_x = Read((operand2 << 8) + operand1 + register_y);
			if(register_x == 0x00)     { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(register_x >= 0x80)     { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 2;
			// TODO: Add 1 to cycle count if crossing page boundary.
			break;
		case 0xBF: IllegalOpcode(0xBF); break;
		case 0xC0:
			operand1 = Read(program_counter + 1);
			result = (register_y - operand1);
			if(register_y >= operand1) { BitSet(register_p, STATUS_BIT_CARRY); }    else { BitClear(register_p, STATUS_BIT_CARRY); }
			if(register_y == operand1) { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(result     >= 0x80)     { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 1;
			break;
		case 0xC1:
			std::cout << "CMP INDX unimp" << '\n';
			program_counter += 1;
			break;
		case 0xC2: IllegalOpcode(0xC2); break;
		case 0xC3: IllegalOpcode(0xC3); break;
		case 0xC4:
			operand1 = Read(program_counter + 1);
			operand2 = Read(operand1);
			result = (register_y - operand2);
			if(register_y >= operand2) { BitSet(register_p, STATUS_BIT_CARRY); }    else { BitClear(register_p, STATUS_BIT_CARRY); }
			if(register_y == operand2) { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(result     >= 0x80)     { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 1;
			break;
		case 0xC5:
			operand1 = Read(program_counter + 1);
			operand2 = Read(operand1);
			result = (register_a - operand2);
			if(register_a >= operand2) { BitSet(register_p, STATUS_BIT_CARRY); }    else { BitClear(register_p, STATUS_BIT_CARRY); }
			if(register_a == operand2) { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(result     >= 0x80)     { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 1;
			break;
		case 0xC6:
			operand1 = Read(program_counter + 1);
			operand2 = Read(operand1);
			operand2--;
			Write(operand1, operand2);
			if(operand2 == 0x00)       { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(operand2 >= 0x80)       { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 1;
			break;
		case 0xC7: IllegalOpcode(0xC7); break;
		case 0xC8:
			register_y++;
			if(register_y == 0x00)     { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(register_y >= 0x80)     { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			break;
		case 0xC9:
			operand1 = Read(program_counter + 1);
			result = (register_a - operand1);
			if(register_a >= operand1) { BitSet(register_p, STATUS_BIT_CARRY); }    else { BitClear(register_p, STATUS_BIT_CARRY); }
			if(register_a == operand1) { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(result     >= 0x80)     { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 1;
			break;
		case 0xCA:
			register_x--;
			if(register_x == 0x00)     { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(register_x >= 0x80)     { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			break;
		case 0xCB: IllegalOpcode(0xCB); break;
		case 0xCC:
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			result = Read((operand2 << 8) + operand1);
			if(register_y >= result)   { BitSet(register_p, STATUS_BIT_CARRY); }    else { BitClear(register_p, STATUS_BIT_CARRY); }
			if(register_y == result)   { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			result = (register_y - result);
			if(result     >= 0x80)     { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 2;
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
			break;
		case 0xCF: IllegalOpcode(0xCF); break;
		case 0xD0:
			operand1 = Read(program_counter + 1);
			program_counter++;
			if(BitCheck(register_p, STATUS_BIT_ZERO) == 0) {
				program_counter += (int8_t)operand1;
				cycles += 1;
				// TODO: Add 1 to cycle count if crossing page boundary.
			}
			break;
		case 0xD1:
			std::cout << "CMP INDY unimp" << '\n';
			program_counter += 1;
			// TODO: Add 1 to cycle count if crossing page boundary.
			break;
		case 0xD2: IllegalOpcode(0xD2); break;
		case 0xD3: IllegalOpcode(0xD3); break;
		case 0xD4: IllegalOpcode(0xD4); break;
		case 0xD5:
			/* Zero Page X Index added here to force wrap around instead of carry in the line after. */
			operand1 = Read(program_counter + 1) + register_x;
			operand2 = Read(operand1);
			result = (register_a - operand2);
			if(register_a >= operand2) { BitSet(register_p, STATUS_BIT_CARRY); }    else { BitClear(register_p, STATUS_BIT_CARRY); }
			if(register_a == operand2) { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(result     >= 0x80)     { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 1;
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
			break;
		case 0xD7: IllegalOpcode(0xD7); break;
		case 0xD8:
			BitClear(register_p, STATUS_BIT_DECIMAL);
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
			// TODO: Add 1 to cycle count if crossing page boundary.
			break;
		case 0xDA: IllegalOpcode(0xDA); break;
		case 0xDB: IllegalOpcode(0xDB); break;
		case 0xDC: IllegalOpcode(0xDC); break;
		case 0xDD:
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			result = Read((operand2 << 8) + operand1 + register_x);
			if(register_a >= result)   { BitSet(register_p, STATUS_BIT_CARRY); }    else { BitClear(register_p, STATUS_BIT_CARRY); }
			if(register_a == result)   { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			result = (register_a - Read((operand2 << 8) + operand1 + register_x));
			if(result     >= 0x80)     { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 2;
			// TODO: Add 1 to cycle count if crossing page boundary.
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
			break;
		case 0xDF: IllegalOpcode(0xDF); break;
		case 0xE0:
			operand1 = Read(program_counter + 1);
			result = (register_x - operand1);
			if(register_x >= operand1) { BitSet(register_p, STATUS_BIT_CARRY); }    else { BitClear(register_p, STATUS_BIT_CARRY); }
			if(register_x == operand1) { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(result     >= 0x80)     { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 1;
			break;
		case 0xE1:
			std::cout << "SBC INDX unimp" << '\n';
			program_counter += 2;
			break;
		case 0xE2: IllegalOpcode(0xE2); break;
		case 0xE3: IllegalOpcode(0xE3); break;
		case 0xE4:
			operand1 = Read(program_counter + 1);
			operand1 = Read(operand1);
			result = (register_x - operand1);
			if(register_x >= operand1) { BitSet(register_p, STATUS_BIT_CARRY); }    else { BitClear(register_p, STATUS_BIT_CARRY); }
			if(register_x == operand1) { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(result     >= 0x80)     { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 1;
			break;
		case 0xE5:
			operand1 = Read(program_counter + 1);
			operand1 = Read(operand1);
			register_a -= (operand1 + !BitCheck(register_p, STATUS_BIT_CARRY)); // TODO: Overflow flag needs to be set.
			if(register_a == 0x00)     { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(register_a >= 0x80)     { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 1;
			break;
		case 0xE6:
			operand1 = Read(program_counter + 1);
			operand2 = Read(operand1);
			operand2++;
			Write(operand1, operand2);
			if(operand2 == 0x00)       { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(operand2 >= 0x80)       { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 1;
			break;
		case 0xE7: IllegalOpcode(0xE7); break;
		case 0xE8:
			register_x++;
			if(register_x == 0x00)     { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(register_x >= 0x80)     { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			break;
		case 0xE9:
			operand1 = Read(program_counter + 1);
			result = (register_a - operand1 - !BitCheck(register_p, STATUS_BIT_CARRY));
			/* Check carry/unsigned overflow. */
			if((result & 0x100) == 0)  { BitSet(register_p, STATUS_BIT_CARRY); }    else { BitClear(register_p, STATUS_BIT_CARRY); }
			/* Check signed overflow. */
			if((register_a ^ result) & (~operand1 ^ result) & 0x80)
									   { BitSet(register_p, STATUS_BIT_OVERFLOW); } else { BitClear(register_p, STATUS_BIT_OVERFLOW); }
			register_a = (uint8_t) result;
			if(register_a == 0x00)     { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(register_a >= 0x80)     { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 1;
			break;
		case 0xEA:
			/* NOP */
			break;
		case 0xEB: IllegalOpcode(0xEB); break;
		case 0xEC:
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			result = Read((operand2 << 8) + operand1);
			if(register_x >= result)   { BitSet(register_p, STATUS_BIT_CARRY); }    else { BitClear(register_p, STATUS_BIT_CARRY); }
			if(register_x == result)   { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			result = (register_x - result);
			if(result     >= 0x80)     { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 2;
			break;
		case 0xED:
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			result = Read((operand2 << 8) + operand1);
			register_a -= (result + !BitCheck(register_p, STATUS_BIT_CARRY)); // TODO: Overflow flag needs to be set.
			if(register_a == 0x00)     { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(register_a >= 0x80)     { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 2;
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
			break;
		case 0xEF: IllegalOpcode(0xEF); break;
		case 0xF0:
			operand1 = Read(program_counter + 1);
			program_counter++;
			if(BitCheck(register_p, STATUS_BIT_ZERO) == 1) {
				program_counter += (int8_t)operand1;
				cycles += 1;
				// TODO: Add 1 to cycle count if crossing page boundary.
			}
			break;
		case 0xF1:
			std::cout << "SBC INDY unimp" << '\n';
			program_counter += 1;
			// TODO: Add 1 to cycle count if crossing page boundary.
			break;
		case 0xF2: IllegalOpcode(0xF2); break;
		case 0xF3: IllegalOpcode(0xF3); break;
		case 0xF4: IllegalOpcode(0xF4); break;
		case 0xF5:
			/* Zero Page X Index added here to force wrap around instead of carry in the line after. */
			operand1 = Read(program_counter + 1) + register_x;
			operand1 = Read(operand1);
			register_a -= (operand1 + !BitCheck(register_p, STATUS_BIT_CARRY)); // TODO: Overflow flag needs to be set.
			if(register_a == 0x00)     { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(register_a >= 0x80)     { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 1;
			break;
		case 0xF6:
			/* Zero Page X Index added here to force wrap around instead of carry in the line after. */
			operand1 = Read(program_counter + 1) + register_x;
			operand2 = Read(operand1);
			operand2++;
			Write(operand1, operand2);
			if(operand2 == 0x00)       { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(operand2 >= 0x80)       { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 1;
			break;
		case 0xF7: IllegalOpcode(0xF7); break;
		case 0xF8:
			BitSet(register_p, STATUS_BIT_DECIMAL);
			break;
		case 0xF9:
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			result = Read((operand2 << 8) + operand1 + register_y);
			register_a -= (result + !BitCheck(register_p, STATUS_BIT_CARRY)); // TODO: Overflow flag needs to be set.
			if(register_a == 0x00)     { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(register_a >= 0x80)     { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 2;
			// TODO: Add 1 to cycle count if crossing page boundary.
			break;
		case 0xFA: IllegalOpcode(0xFA); break;
		case 0xFB: IllegalOpcode(0xFB); break;
		case 0xFC: IllegalOpcode(0xFC); break;
		case 0xFD:
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			result = (register_a - Read((operand2 << 8) + operand1 + register_x) - !BitCheck(register_p, STATUS_BIT_CARRY));
			/* Check carry/unsigned overflow. */
			if((result & 0x100) == 0)  { BitSet(register_p, STATUS_BIT_CARRY); }    else { BitClear(register_p, STATUS_BIT_CARRY); }
			/* Check signed overflow. */
			if((register_a ^ result) & (~Read((operand2 << 8) + operand1 + register_x) ^ result) & 0x80)
									   { BitSet(register_p, STATUS_BIT_OVERFLOW); } else { BitClear(register_p, STATUS_BIT_OVERFLOW); }
			register_a = (uint8_t) result;
			if(register_a == 0x00)     { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(register_a >= 0x80)     { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
			program_counter += 2;
			// TODO: Add 1 to cycle count if crossing page boundary.
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
			break;
		case 0xFF: IllegalOpcode(0xFF); break;
		default:
			std::cout << "Reached normally unreachable default in switch statement." << '\n';
			while (1);
			break;
	}

	// TODO: Add structure with instruction size to CPU class, then add to pc here. Remove "increment_pc" variable below.
	//program_counter += instruction_sizes[instruction];
	cycles += cycle_sizes[instruction];

	if(increment_pc) {
		program_counter++;
	} else {
		increment_pc = true;
	}
}