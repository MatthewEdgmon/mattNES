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

void CPU::Step() {

	if(halted) {
		return;
	}

	instruction = Read(program_counter);

	uint16_t old_pc = 0;
	uint8_t result = 0;
	uint16_t result16 = 0;

	switch(instruction) {
		case 0x00:
			Interrupt(INTERRUPT_BRK);
			break;
		case 0x01:
			/* (d,X) */
			operand1 = Read(program_counter + 1);
			operand2 = Read(static_cast<uint8_t>(operand1 + register_x + 1));
			operand1 = Read(static_cast<uint8_t>(operand1 + register_x));
			register_a |= Read((operand2 << 8) + operand1);
			UpdateZeroNegative(register_a);
			program_counter += 1;
			break;
		case 0x02:
			illegal_opcode_triggered = true;
			halted = true;
			/* STP */
			break;
		case 0x03:
			illegal_opcode_triggered = true;
			/* (d,X) */
			operand1 = Read(program_counter + 1);
			operand2 = Read(static_cast<uint8_t>(operand1 + register_x + 1));
			operand1 = Read(static_cast<uint8_t>(operand1 + register_x));
			/* ASL */
			/* Check if bit 7 is set to preserve value. */
			SetFlag(STATUS_BIT_CARRY, (Read((operand2 << 8) + operand1) & 0x80));
			result = (Read((operand2 << 8) + operand1) << 1);
			Write(Read((operand2 << 8) + operand1), result);
			UpdateZeroNegative(result);
			/* ORA */
			register_a |= Read((operand2 << 8) + operand1);
			UpdateZeroNegative(register_a);
			program_counter += 1;
			break;
		case 0x04:
			/* NOP */
			program_counter += 1;
			break;
		case 0x05:
			operand1 = Read(program_counter + 1);
			operand1 = Read(operand1);
			register_a |= operand1;
			UpdateZeroNegative(register_a);
			program_counter += 1;
			break;
		case 0x06:
			operand1 = Read(program_counter + 1);
			operand2 = Read(operand1);
			/* Check if bit 7 is set to preserve value. */
			SetFlag(STATUS_BIT_CARRY, (operand2 & 0x80));
			operand2 <<= 1;
			Write(operand1, operand2);
			UpdateZeroNegative(operand2);
			program_counter += 1;
			break;
		case 0x07:
			illegal_opcode_triggered = true;
			/* d */
			operand1 = Read(program_counter + 1);
			operand2 = Read(operand1);
			/* ASL */
			/* Check if bit 7 is set to preserve value. */
			SetFlag(STATUS_BIT_CARRY, (operand2 & 0x80));
			operand2 <<= 1;
			Write(operand1, operand2);
			UpdateZeroNegative(operand2);
			/* ORA */
			register_a |= operand2;
			UpdateZeroNegative(register_a);
			program_counter += 1;
			break;
		case 0x08:
			/* When pushing the flag register, bits 4 and 5 are set in the value pushed. */
			result = register_p | 0x30;
			Push(result);
			break;
		case 0x09:
			operand1 = Read(program_counter + 1);
			register_a |= operand1;
			UpdateZeroNegative(register_a);
			program_counter += 1;
			break;
		case 0x0A:
			/* Check if bit 7 is set to preserve value. */
			SetFlag(STATUS_BIT_CARRY, (register_a & 0x80));
			register_a <<= 1;
			UpdateZeroNegative(register_a);
			break;
		case 0x0B:
			illegal_opcode_triggered = true;
			std::cout << "ANC #i unimp\n";
			program_counter += 1;
			break;
		case 0x0C:
			illegal_opcode_triggered = true;
			/* NOP */
			program_counter += 2;
			break;
		case 0x0D:
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			register_a |= Read((operand2 << 8) + operand1);
			UpdateZeroNegative(register_a);
			program_counter += 2;
			break;
		case 0x0E:
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			result = Read((operand2 << 8) + operand1);
			/* Check if bit 7 is set to preserve value. */
			SetFlag(STATUS_BIT_CARRY, (result & 0x80));
			result <<= 1;
			Write((operand2 << 8) + operand1, result);
			UpdateZeroNegative(result);
			program_counter += 2;
			break;
		case 0x0F:
			illegal_opcode_triggered = true;
			/* ABS */
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			/* ASL */
			/* Check if bit 7 is set to preserve value. */
			SetFlag(STATUS_BIT_CARRY, (Read((operand2 << 8) + operand1) & 0x80));
			result = (Read((operand2 << 8) + operand1) << 1);
			Write((operand2 << 8) + operand1, result);
			UpdateZeroNegative(result);
			/* ORA */
			register_a |= result;
			UpdateZeroNegative(register_a);
			program_counter += 2;
			break;
		case 0x10:
			operand1 = Read(program_counter + 1);
			program_counter++;
			if(BitCheck(register_p, STATUS_BIT_NEGATIVE) == 0) {
				CheckPageCross(program_counter, (program_counter + static_cast<int8_t>(operand1)), 1);
				program_counter += static_cast<int8_t>(operand1);
				cycles += 1;
			}
			break;
		case 0x11:
			/* Fetch address from zero page, add register Y to that address. */
			operand1 = Read(program_counter + 1);
			operand2 = Read(operand1 + 1);
			operand1 = Read(operand1);
			register_a |= Read((operand2 << 8) + operand1 + register_y);
			UpdateZeroNegative(register_a);
			program_counter += 1;
			// TODO: Add 1 to cycle count if crossing page boundary.
			break;
		case 0x12:
			illegal_opcode_triggered = true;
			halted = true;
			/* STP */
			break;
		case 0x13:
			illegal_opcode_triggered = true;
			/* (d),Y */
			operand1 = Read(program_counter + 1);
			operand2 = Read(operand1 + 1);
			operand1 = Read(operand1);
			/* ASL */
			/* Check if bit 7 is set to preserve value. */
			SetFlag(STATUS_BIT_CARRY, (Read((operand2 << 8) + operand1 + register_y) & 0x80));
			result = (Read((operand2 << 8) + operand1 + register_y) << 1);
			Write(Read((operand2 << 8) + operand1 + register_y), result);
			UpdateZeroNegative(result);
			/* ORA */
			register_a |= Read((operand2 << 8) + operand1 + register_y);
			UpdateZeroNegative(register_a);
			program_counter += 1;
			break;
		case 0x14:
			illegal_opcode_triggered = true;
			/* NOP */
			program_counter += 1;
			break;
		case 0x15:
			/* Zero Page X Index added here to force wrap around instead of carry in the line after. */
			operand1 = (Read(program_counter + 1) + register_x);
			operand1 = Read(operand1);
			register_a |= operand1;
			UpdateZeroNegative(register_a);
			program_counter += 1;
			break;
		case 0x16:
			/* Zero Page X Index added here to force wrap around instead of carry in the line after. */
			operand1 = (Read(program_counter + 1) + register_x);
			operand2 = Read(operand1);
			/* Check if bit 7 is set to preserve value. */
			SetFlag(STATUS_BIT_CARRY, (operand2 & 0x80));
			operand2 <<= 1;
			Write(operand1, operand2);
			UpdateZeroNegative(operand2);
			program_counter += 1;
			break;
		case 0x17:
			illegal_opcode_triggered = true;
			/* d,X */
			operand1 = Read(program_counter + 1) + register_x;
			operand2 = Read(operand1);
			/* ASL */
			/* Check if bit 7 is set to preserve value. */
			SetFlag(STATUS_BIT_CARRY, (operand2 & 0x80));
			operand2 <<= 1;
			Write(operand1, operand2);
			UpdateZeroNegative(operand2);
			/* ORA */
			register_a |= operand2;
			UpdateZeroNegative(register_a);
			program_counter += 1;
			break;
		case 0x18:
			BitClear(register_p, STATUS_BIT_CARRY);
			break;
		case 0x19:
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			register_a |= Read((operand2 << 8) + operand1 + register_y);
			UpdateZeroNegative(register_a);
			program_counter += 2;
			// TODO: Add 1 to cycle count if crossing page boundary.
			break;
		case 0x1A:
			illegal_opcode_triggered = true;
			/* NOP */
			break;
		case 0x1B:
			illegal_opcode_triggered = true;
			/* ABS,Y */
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			/* ASL */
			/* Check if bit 7 is set to preserve value. */
			SetFlag(STATUS_BIT_CARRY, (Read((operand2 << 8) + operand1 + register_y) & 0x80));
			result = (Read((operand2 << 8) + operand1 + register_y) << 1);
			Write((operand2 << 8) + operand1 + register_y, result);
			UpdateZeroNegative(result);
			/* ORA */
			register_a |= result;
			UpdateZeroNegative(register_a);
			program_counter += 2;
			break;
		case 0x1C:
			illegal_opcode_triggered = true;
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			Read((operand2 << 8) + operand1 + register_x);
			/* NOP */
			// TODO: Add 1 to cycle count if crossing page boundary.
			break;
		case 0x1D:
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			register_a |= Read((operand2 << 8) + operand1 + register_x);
			UpdateZeroNegative(register_a);
			program_counter += 1;
			// TODO: Add 1 to cycle count if crossing page boundary.
			break;
		case 0x1E:
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			result = Read((operand2 << 8) + operand1 + register_x);
			/* Check if bit 7 is set to preserve value. */
			SetFlag(STATUS_BIT_CARRY, (result & 0x80));
			result <<= 1;
			Write((operand2 << 8) + operand1 + register_x, result);
			UpdateZeroNegative(result);
			program_counter += 2;
			break;
		case 0x1F:
			illegal_opcode_triggered = true;
			/* ABS,X */
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			/* ASL */
			/* Check if bit 7 is set to preserve value. */
			SetFlag(STATUS_BIT_CARRY, (Read((operand2 << 8) + operand1 + register_x) & 0x80));
			result = (Read((operand2 << 8) + operand1 + register_x) << 1);
			Write((operand2 << 8) + operand1 + register_x, result);
			UpdateZeroNegative(result);
			/* ORA */
			register_a |= result;
			UpdateZeroNegative(register_a);
			program_counter += 2;
			break;
		case 0x20:
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			Push((program_counter + 2) >> 8);
			Push((program_counter + 2));
			program_counter = (operand2 << 8) + operand1;
			increment_pc = false;
			break;
		case 0x21:
			/* (d,X) */
			operand1 = Read(program_counter + 1);
			operand2 = Read(static_cast<uint8_t>(operand1 + register_x + 1));
			operand1 = Read(static_cast<uint8_t>(operand1 + register_x));
			register_a &= Read((operand2 << 8) + operand1);
			UpdateZeroNegative(register_a);
			program_counter += 1;
			break;
		case 0x22:
			illegal_opcode_triggered = true;
			halted = true;
			/* STP */
			break;
		case 0x23:
			illegal_opcode_triggered = true;
			/* (d,X) */
			operand1 = Read(program_counter + 1);
			operand2 = Read(static_cast<uint8_t>(operand1 + register_x + 1));
			operand1 = Read(static_cast<uint8_t>(operand1 + register_x));
			/* ROL */ // TODO: Go through ROL and ROR and replace it with this.
			/* Check if carry flag is set to preserve value. */
			result = BitCheck(register_p, STATUS_BIT_CARRY);
			/* Check if bit 7 is set to preserve value. */
			SetFlag(STATUS_BIT_CARRY, (Read((operand2 << 8) + operand1) & 0x80));
			/* Perform shift, add old bit 1. */
			result = (Read((operand2 << 8) + operand1) << 1) + result;
			/* Finally write value. */
			Write((operand2 << 8) + operand1, result);
			UpdateZeroNegative(result);
			/* AND */
			register_a &= result;
			UpdateZeroNegative(register_a);
			program_counter += 1;
			break;
		case 0x24:
			operand1 = Read(program_counter + 1);
			operand1 = Read(operand1);
			SetFlag(STATUS_BIT_ZERO, ((operand1 & register_a) == 0));
			SetFlag(STATUS_BIT_OVERFLOW, BitCheck(operand1, 6));
			SetFlag(STATUS_BIT_NEGATIVE, BitCheck(operand1, 7));
			program_counter += 1;
			break;
		case 0x25:
			operand1 = Read(program_counter + 1);
			operand1 = Read(operand1);
			register_a &= operand1;
			UpdateZeroNegative(register_a);
			program_counter += 1;
			break;
		case 0x26:
			/* d */
			operand1 = Read(program_counter + 1);
			operand2 = Read(operand1);
			/* Check if carry flag is set to preserve value. */
			result = BitCheck(register_p, STATUS_BIT_CARRY);
			/* Check if bit 7 is set to preserve value. */
			SetFlag(STATUS_BIT_CARRY, (operand2 & 0x80));
			/* Perform shift, add old bit 1. */
			result += (operand2 << 1);
			/* Finally write value. */
			Write(operand1, result);
			UpdateZeroNegative(result);
			program_counter += 1;
			break;
		case 0x27:
			illegal_opcode_triggered = true;
			/* d */
			operand1 = Read(program_counter + 1);
			operand2 = Read(operand1);
			/* ROL */
			/* Check if carry flag is set to preserve value. */
			result = BitCheck(register_p, STATUS_BIT_CARRY);
			/* Check if bit 7 is set to preserve value. */
			SetFlag(STATUS_BIT_CARRY, (operand2 & 0x80));
			/* Perform shift, add old bit 1. */
			result += (operand2 << 1);
			/* Finally write value. */
			Write(operand1, result);
			UpdateZeroNegative(result);
			/* AND */
			register_a &= result;
			UpdateZeroNegative(register_a);
			program_counter += 1;
			break;
		case 0x28:
			/* When pulling the flag register, bits 4 and 5 are set in the value pulled. */
			result = Pop();
			// TODO: On real hardware, bits 4 and 5 are ignored in the value popped.
			//result &= 0xCF;
			BitClear(result, 4);
			BitSet(result, 5);
			register_p = result;
			break;
		case 0x29:
			operand1 = Read(program_counter + 1);
			register_a &= operand1;
			UpdateZeroNegative(register_a);
			program_counter += 1;
			break;
		case 0x2A:
			/* ACC */
			/* Check if carry flag is set to preserve value. */
			result = BitCheck(register_p, STATUS_BIT_CARRY);
			/* Check if bit 7 is set to preserve value. */
			SetFlag(STATUS_BIT_CARRY, (register_a & 0x80));
			/* Perform shift, add old bit 1. */
			result += (register_a << 1);
			/* Finally write value. */
			register_a = result;
			UpdateZeroNegative(register_a);
			break;
		case 0x2B:
			illegal_opcode_triggered = true;
			std::cout << "ANC #i unimp\n";
			program_counter += 1;
			break;
		case 0x2C:
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			operand1 = Read((operand2 << 8) + operand1);
			SetFlag(STATUS_BIT_ZERO, ((operand1 & register_a) == 0));
			SetFlag(STATUS_BIT_OVERFLOW, BitCheck(operand1, 6));
			SetFlag(STATUS_BIT_NEGATIVE, BitCheck(operand1, 7));
			program_counter += 2;
			break;
		case 0x2D:
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			register_a &= Read((operand2 << 8) + operand1);
			UpdateZeroNegative(register_a);
			program_counter += 2;
			break;
		case 0x2E:
			/* ABS */
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			/* Check if carry flag is set to preserve value. */
			result = BitCheck(register_p, STATUS_BIT_CARRY);
			/* Check if bit 7 is set to preserve value. */
			SetFlag(STATUS_BIT_CARRY, (Read((operand2 << 8) + operand1) & 0x80));
			/* Perform shift, add old bit 1. */
			result += (Read((operand2 << 8) + operand1) << 1);
			/* Finally write value. */
			Write((operand2 << 8) + operand1, result);
			UpdateZeroNegative(result);
			program_counter += 2;
			break;
		case 0x2F:
			illegal_opcode_triggered = true;
			/* ABS */
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			/* ROL */
			/* Check if carry flag is set to preserve value. */
			result = BitCheck(register_p, STATUS_BIT_CARRY);
			/* Check if bit 7 is set to preserve value. */
			SetFlag(STATUS_BIT_CARRY, (Read((operand2 << 8) + operand1) & 0x80));
			/* Perform shift, add old bit 1. */
			result += (Read((operand2 << 8) + operand1) << 1);
			/* Finally write value. */
			Write((operand2 << 8) + operand1, result);
			UpdateZeroNegative(result);
			/* AND */
			register_a &= result;
			UpdateZeroNegative(register_a);
			program_counter += 2;
			break;
		case 0x30:
			operand1 = Read(program_counter + 1);
			program_counter++;
			if(BitCheck(register_p, STATUS_BIT_NEGATIVE)) {
				CheckPageCross(program_counter, (program_counter + static_cast<int8_t>(operand1)), 1);
				program_counter += static_cast<int8_t>(operand1);
				cycles += 1;
			}
			break;
		case 0x31:
			/* Fetch address from zero page, add register Y to that address. */
			operand1 = Read(program_counter + 1);
			operand2 = Read(operand1 + 1);
			operand1 = Read(operand1);
			register_a &= Read((operand2 << 8) + operand1 + register_y);
			UpdateZeroNegative(register_a);
			program_counter += 1;
			// TODO: Add 1 to cycle count if crossing page boundary.
			break;
		case 0x32:
			illegal_opcode_triggered = true;
			halted = true;
			/* STP */
			break;
		case 0x33:
			illegal_opcode_triggered = true;
			/* (d),Y */
			operand1 = Read(program_counter + 1);
			operand2 = Read(operand1 + 1);
			operand1 = Read(operand1);
			/* ROL */
			/* Check if carry flag is set to preserve value. */
			result = BitCheck(register_p, STATUS_BIT_CARRY);
			/* Check if bit 7 is set to preserve value. */
			SetFlag(STATUS_BIT_CARRY, (Read((operand2 << 8) + operand1 + register_y) & 0x80));
			/* Perform shift, add old bit 1. */
			result += (Read((operand2 << 8) + operand1 + register_y) << 1);
			/* Finally write value. */
			Write((operand2 << 8) + operand1 + register_y, result);
			UpdateZeroNegative(result);
			/* AND */
			register_a &= result;
			UpdateZeroNegative(register_a);
			program_counter += 1;
			break;
		case 0x34:
			illegal_opcode_triggered = true;
			/* NOP */
			program_counter += 1;
			break;
		case 0x35:
			/* Zero Page X Index added here to force wrap around instead of carry in the line after. */
			operand1 = Read(program_counter + 1) + register_x;
			operand2 = Read(operand1);
			register_a &= operand2;
			UpdateZeroNegative(register_a);
			program_counter += 1;
			break;
		case 0x36:
			/* d,X */
			operand1 = Read(program_counter + 1) + register_x;
			operand2 = Read(operand1);
			/* Check if carry flag is set to preserve value. */
			result = BitCheck(register_p, STATUS_BIT_CARRY);
			/* Check if bit 7 is set to preserve value. */
			SetFlag(STATUS_BIT_CARRY, (operand2 & 0x80));
			/* Perform shift, add old bit 1. */
			result += (operand2 << 1);
			/* Finally write value. */
			Write(operand1, result);
			UpdateZeroNegative(result);
			program_counter += 1;
			break;
		case 0x37:
			illegal_opcode_triggered = true;
			/* d,X */
			operand1 = Read(program_counter + 1) + register_x;
			operand2 = Read(operand1);
			/* ROL */
			/* Check if carry flag is set to preserve value. */
			result = BitCheck(register_p, STATUS_BIT_CARRY);
			/* Check if bit 7 is set to preserve value. */
			SetFlag(STATUS_BIT_CARRY, (operand2 & 0x80));
			/* Perform shift, add old bit 1. */
			result += (operand2 << 1);
			/* Finally write value. */
			Write(operand1, result);
			UpdateZeroNegative(result);
			/* AND */
			register_a &= result;
			UpdateZeroNegative(register_a);
			program_counter += 1;
			break;
		case 0x38:
			BitSet(register_p, STATUS_BIT_CARRY);
			break;
		case 0x39:
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			register_a &= Read((operand2 << 8) + operand1 + register_y);
			UpdateZeroNegative(register_a);
			program_counter += 2;
			// TODO: Add 1 to cycle count if crossing page boundary.
			break;
		case 0x3A:
			illegal_opcode_triggered = true;
			/* NOP */
			break;
		case 0x3B:
			illegal_opcode_triggered = true;
			/* ABS,Y */
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			/* ROL */
			/* Check if carry flag is set to preserve value. */
			result = BitCheck(register_p, STATUS_BIT_CARRY);
			/* Check if bit 7 is set to preserve value. */
			SetFlag(STATUS_BIT_CARRY, (Read((operand2 << 8) + operand1 + register_y) & 0x80));
			/* Perform shift, add old bit 1. */
			result += (Read((operand2 << 8) + operand1 + register_y) << 1);
			/* Finally write value. */
			Write((operand2 << 8) + operand1 + register_y, result);
			UpdateZeroNegative(result);
			/* AND */
			register_a &= result;
			UpdateZeroNegative(register_a);
			program_counter += 2;
			break;
		case 0x3C:
			illegal_opcode_triggered = true;
			/* NOP */
			program_counter += 2;
			break;
		case 0x3D:
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			register_a &= Read((operand2 << 8) + operand1 + register_x);
			UpdateZeroNegative(register_a);
			program_counter += 2;
			// TODO: Add 1 to cycle count if crossing page boundary.
			break;
		case 0x3E:
			/* ABS,X */
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			/* Check if carry flag is set to preserve value. */
			result = BitCheck(register_p, STATUS_BIT_CARRY);
			/* Check if bit 7 is set to preserve value. */
			SetFlag(STATUS_BIT_CARRY, (Read((operand2 << 8) + operand1 + register_x) & 0x80));
			/* Perform shift, add old bit 1. */
			result += (Read((operand2 << 8) + operand1 + register_x) << 1);
			/* Finally write value. */
			Write((operand2 << 8) + operand1 + register_x, result);
			UpdateZeroNegative(result);
			program_counter += 2;
			break;
		case 0x3F:
			illegal_opcode_triggered = true;
			/* ABS,X */
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			/* ROL */
			/* Check if carry flag is set to preserve value. */
			result = BitCheck(register_p, STATUS_BIT_CARRY);
			/* Check if bit 7 is set to preserve value. */
			SetFlag(STATUS_BIT_CARRY, (Read((operand2 << 8) + operand1 + register_x) & 0x80));
			/* Perform shift, add old bit 1. */
			result += (Read((operand2 << 8) + operand1 + register_x) << 1);
			/* Finally write value. */
			Write((operand2 << 8) + operand1 + register_x, result);
			UpdateZeroNegative(result);
			/* AND */
			register_a &= result;
			UpdateZeroNegative(register_a);
			program_counter += 2;
			break;
		case 0x40:
			/* Pop flags, low byte, and high byte in that order. */
			result = Pop();
			// TODO: On real hardware, bits 4 and 5 are ignored in the value popped.
			//result ^= 0x30;
			BitClear(result, 4);
			BitSet(result, 5);
			register_p = result;
			program_counter  = Pop();
			program_counter += Pop() << 8;
			increment_pc = false;
			break;
		case 0x41:
			/* (d,X) */
			operand1 = Read(program_counter + 1);
			operand2 = Read(static_cast<uint8_t>(operand1 + register_x + 1));
			operand1 = Read(static_cast<uint8_t>(operand1 + register_x));
			register_a ^= Read((operand2 << 8) + operand1);
			UpdateZeroNegative(register_a);
			program_counter += 1;
			break;
		case 0x42:
			illegal_opcode_triggered = true;
			halted = true;
			/* STP */
			break;
		case 0x43:
			illegal_opcode_triggered = true;
			/* (d,X) */
			operand1 = Read(program_counter + 1);
			operand2 = Read(static_cast<uint8_t>(operand1 + register_x + 1));
			operand1 = Read(static_cast<uint8_t>(operand1 + register_x));
			/* LSR */
			/* Check if bit 0 is set to preserve value. */
			SetFlag(STATUS_BIT_CARRY, (Read((operand2 << 8) + operand1) & 0x01));
			result = (Read((operand2 << 8) + operand1) >> 1);
			Write((operand2 << 8) + operand1, result);
			UpdateZeroNegative(result);
			/* EOR */
			register_a ^= result;
			UpdateZeroNegative(register_a);
			program_counter += 1;
			break;
		case 0x44:
			illegal_opcode_triggered = true;
			/* NOP */
			program_counter += 1;
			break;
		case 0x45:
			operand1 = Read(program_counter + 1);
			operand1 = Read(operand1);
			register_a ^= operand1;
			UpdateZeroNegative(register_a);
			program_counter += 1;
			break;
		case 0x46:
			operand1 = Read(program_counter + 1);
			operand2 = Read(operand1);
			/* Check if bit 0 is set to preserve value. */
			SetFlag(STATUS_BIT_CARRY, (operand2 & 0x01));
			operand2 >>= 1;
			Write(operand1, operand2);
			UpdateZeroNegative(operand2);
			program_counter += 1;
			break;
		case 0x47:
			illegal_opcode_triggered = true;
			/* d */
			operand1 = Read(program_counter + 1);
			operand2 = Read(operand1);
			/* LSR */
			/* Check if bit 0 is set to preserve value. */
			SetFlag(STATUS_BIT_CARRY, (operand2 & 0x01));
			result = (operand2 >> 1);
			Write(operand1, result);
			UpdateZeroNegative(result);
			/* EOR */
			register_a ^= result;
			UpdateZeroNegative(register_a);
			program_counter += 1;
			break;
		case 0x48:
			Push(register_a);
			break;
		case 0x49:
			operand1 = Read(program_counter + 1);
			register_a ^= operand1;
			UpdateZeroNegative(register_a);
			program_counter += 1;
			break;
		case 0x4A:
			/* Check if bit 0 is set to preserve value. */
			SetFlag(STATUS_BIT_CARRY, (register_a & 0x01));
			register_a >>= 1;
			UpdateZeroNegative(register_a);
			break;
		case 0x4B:
			illegal_opcode_triggered = true;
			break;
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
			UpdateZeroNegative(register_a);
			program_counter += 2;
			break;
		case 0x4E:
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			result = Read((operand2 << 8) + operand1);
			/* Check if bit 0 is set to preserve value. */
			SetFlag(STATUS_BIT_CARRY, (result & 0x01));
			result >>= 1;
			Write((operand2 << 8) + operand1, result);
			UpdateZeroNegative(result);
			program_counter += 2;
			break;
		case 0x4F:
			illegal_opcode_triggered = true;
			/* ABS */
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			/* LSR */
			/* Check if bit 0 is set to preserve value. */
			SetFlag(STATUS_BIT_CARRY, (Read((operand2 << 8) + operand1) & 0x01));
			result = (Read((operand2 << 8) + operand1) >> 1);
			Write((operand2 << 8) + operand1, result);
			UpdateZeroNegative(result);
			/* EOR */
			register_a ^= result;
			UpdateZeroNegative(register_a);
			program_counter += 2;
			break;
		case 0x50:
			operand1 = Read(program_counter + 1);
			program_counter++;
			if(!BitCheck(register_p, STATUS_BIT_OVERFLOW)) {
				CheckPageCross(program_counter, (program_counter + static_cast<int8_t>(operand1)), 1);
				program_counter += static_cast<int8_t>(operand1);
				cycles += 1;
			}
			break;
		case 0x51:
			/* Fetch address from zero page, add register Y to that address. */
			operand1 = Read(program_counter + 1);
			operand2 = Read(operand1 + 1);
			operand1 = Read(operand1);
			register_a ^= Read((operand2 << 8) + operand1 + register_y);
			UpdateZeroNegative(register_a);
			program_counter += 1;
			// TODO: Add 1 to cycle count if crossing page boundary.
			break;
		case 0x52:
			illegal_opcode_triggered = true;
			halted = true;
			/* STP */
			break;
		case 0x53:
			illegal_opcode_triggered = true;
			/* (d),Y */
			operand1 = Read(program_counter + 1);
			operand2 = Read(operand1 + 1);
			operand1 = Read(operand1);
			/* LSR */
			/* Check if bit 0 is set to preserve value. */
			SetFlag(STATUS_BIT_CARRY, (Read((operand2 << 8) + operand1 + register_y) & 0x01));
			result = (Read((operand2 << 8) + operand1 + register_y) >> 1);
			Write((operand2 << 8) + operand1 + register_y, result);
			UpdateZeroNegative(result);
			/* EOR */
			register_a ^= result;
			UpdateZeroNegative(register_a);
			program_counter += 1;
			break;
		case 0x54:
			illegal_opcode_triggered = true;
			/* NOP */
			program_counter += 1;
			break;
		case 0x55:
			/* Zero Page X Index added here to force wrap around here instead of carry in the line after. */
			operand1 = Read(program_counter + 1) + register_x;
			operand1 = Read(operand1);
			register_a ^= operand1;
			UpdateZeroNegative(register_a);
			program_counter += 1;
			break;
		case 0x56:
			/* Zero Page X Index added here to force wrap around here instead of carry in the line after. */
			operand1 = Read(program_counter + 1) + register_x;
			operand2 = Read(operand1);
			/* Check if bit 0 is set to preserve value. */
			SetFlag(STATUS_BIT_CARRY, (operand2 & 0x01));
			operand2 >>= 1;
			Write(operand1, operand2);
			UpdateZeroNegative(operand2);
			program_counter += 1;
			break;
		case 0x57:
			illegal_opcode_triggered = true;
			/* d,X */
			operand1 = Read(program_counter + 1) + register_x;
			operand2 = Read(operand1);
			/* LSR */
			/* Check if bit 0 is set to preserve value. */
			SetFlag(STATUS_BIT_CARRY, (operand2 & 0x01));
			result = (operand2 >> 1);
			Write(operand1, result);
			UpdateZeroNegative(result);
			/* EOR */
			register_a ^= result;
			UpdateZeroNegative(register_a);
			program_counter += 1;
			break;
		case 0x58:
			SetFlag(STATUS_BIT_INTERRUPT_DISABLE, false);
			break;
		case 0x59:
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			register_a ^= Read((operand2 << 8) + operand1 + register_y);
			UpdateZeroNegative(register_a);
			program_counter += 2;
			// TODO: Add 1 to cycle count if crossing page boundary.
			break;
		case 0x5A:
			illegal_opcode_triggered = true;
			/* NOP */
			break;
		case 0x5B:
			illegal_opcode_triggered = true;
			/* ABS,Y */
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			/* LSR */
			/* Check if bit 0 is set to preserve value. */
			SetFlag(STATUS_BIT_CARRY, (Read((operand2 << 8) + operand1 + register_y) & 0x01));
			result = (Read((operand2 << 8) + operand1 + register_y) >> 1);
			Write((operand2 << 8) + operand1 + register_y, result);
			UpdateZeroNegative(result);
			/* EOR */
			register_a ^= result;
			UpdateZeroNegative(register_a);
			program_counter += 2;
			break;
		case 0x5C:
			illegal_opcode_triggered = true;
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			Read((operand2 << 8) + operand1 + register_x);
			/* NOP */
			// TODO: Add 1 to cycle count if crossing page boundary.
			break;
		case 0x5D:
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			register_a ^= Read((operand2 << 8) + operand1 + register_x);
			UpdateZeroNegative(register_a);
			program_counter += 2;
			// TODO: Add 1 to cycle count if crossing page boundary.
			break;
		case 0x5E:
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			result = Read((operand2 << 8) + operand1 + register_x);
			/* Check if bit 0 is set to preserve value. */
			SetFlag(STATUS_BIT_CARRY, (result & 0x1));
			result >>= 1;
			Write(((operand2 << 8) + operand1 + register_x), result);
			UpdateZeroNegative(result);
			program_counter += 2;
			break;
		case 0x5F:
			/* ABS,X */
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			/* LSR */
			/* Check if bit 0 is set to preserve value. */
			SetFlag(STATUS_BIT_CARRY, (Read((operand2 << 8) + operand1 + register_x) & 0x01));
			result = (Read((operand2 << 8) + operand1 + register_x) >> 1);
			Write((operand2 << 8) + operand1 + register_x, result);
			UpdateZeroNegative(result);
			/* EOR */
			register_a ^= result;
			UpdateZeroNegative(register_a);
			program_counter += 2;
			break;
		case 0x60:
			program_counter =  Pop();
			program_counter |= Pop() << 8;
			break;
		case 0x61:
			/* (d,X) */
			operand1 = Read(program_counter + 1);
			operand2 = Read(static_cast<uint8_t>(operand1 + register_x + 1));
			operand1 = Read(static_cast<uint8_t>(operand1 + register_x));
			result16 = register_a + Read((operand2 << 8) + operand1) + BitCheck(register_p, STATUS_BIT_CARRY);
			/* Check carry/unsigned overflow. */
			SetFlag(STATUS_BIT_CARRY, (result16 & 0x100));
			/* Check signed overflow. */
			SetFlag(STATUS_BIT_OVERFLOW, ((register_a ^ result16) & (Read((operand2 << 8) + operand1) ^ result16) & 0x80));
			register_a = static_cast<uint8_t>(result16);
			UpdateZeroNegative(register_a);
			program_counter += 1;
			break;
		case 0x62:
			illegal_opcode_triggered = true;
			halted = true;
			/* STP */
			break;
		case 0x63:
			illegal_opcode_triggered = true;
			/* (d,X) */
			operand1 = Read(program_counter + 1);
			operand2 = Read(static_cast<uint8_t>(operand1 + register_x + 1));
			operand1 = Read(static_cast<uint8_t>(operand1 + register_x));
			/* ROR */
			/* Check if carry flag is set to preserve value. */
			result = (BitCheck(register_p, STATUS_BIT_CARRY) << 7);
			/* Check if bit 0 is set to preserve value. */
			SetFlag(STATUS_BIT_CARRY, (Read((operand2 << 8) + operand1) & 0x01));
			/* Perform shift, add old carry bit. */
			result += (Read((operand2 << 8) + operand1) >> 1);
			/* Finally write value. */
			Write((operand2 << 8) + operand1, result);
			UpdateZeroNegative(result);
			/* ADC */
			result16 = register_a + result + BitCheck(register_p, STATUS_BIT_CARRY);
			/* Check carry/unsigned overflow. */
			SetFlag(STATUS_BIT_CARRY, (result16 & 0x100));
			/* Check signed overflow. */
			SetFlag(STATUS_BIT_OVERFLOW, ((register_a ^ result16) & (result ^ result16) & 0x80));
			register_a = static_cast<uint8_t>(result16);
			UpdateZeroNegative(register_a);
			program_counter += 1;
			break;
		case 0x64:
			illegal_opcode_triggered = true;
			/* NOP */
			program_counter += 1;
			break;
		case 0x65:
			/* d */
			operand1 = Read(program_counter + 1);
			operand2 = Read(operand1);
			result16 = register_a + operand2 + BitCheck(register_p, STATUS_BIT_CARRY);
			/* Check carry/unsigned overflow. */
			SetFlag(STATUS_BIT_CARRY, (result16 & 0x100));
			/* Check signed overflow. */
			SetFlag(STATUS_BIT_OVERFLOW, ((register_a ^ result16) & (operand1 ^ result16) & 0x80));
			register_a = static_cast<uint8_t>(result16); 
			UpdateZeroNegative(register_a);
			program_counter += 1;
			break;
		case 0x66:
			operand1 = Read(program_counter + 1);
			operand2 = Read(operand1);
			/* Check if carry flag is set to preserve value. */
			result = (BitCheck(register_p, STATUS_BIT_CARRY) << 7);
			/* Check if bit 0 is set to preserve value. */
			SetFlag(STATUS_BIT_CARRY, (operand2 & 0x01));
			/* Perform shift, add old carry bit. */
			result += (operand2 >> 1);
			/* Finally write value. */
			Write(operand1, result);
			UpdateZeroNegative(result);
			program_counter += 1;
			break;
		case 0x67:
			illegal_opcode_triggered = true;
			/* d */
			operand1 = Read(program_counter + 1);
			operand2 = Read(operand1);
			/* ROR */
			/* Check if carry flag is set to preserve value. */
			result = (BitCheck(register_p, STATUS_BIT_CARRY) << 7);
			/* Check if bit 0 is set to preserve value. */
			SetFlag(STATUS_BIT_CARRY, (operand2 & 0x01));
			/* Perform shift, add old carry bit. */
			result += (operand2 >> 1);
			/* Finally write value. */
			Write(operand1, result);
			UpdateZeroNegative(result);
			/* ADC */
			result16 = register_a + result + BitCheck(register_p, STATUS_BIT_CARRY);
			/* Check carry/unsigned overflow. */
			SetFlag(STATUS_BIT_CARRY, (result16 & 0x100));
			/* Check signed overflow. */
			SetFlag(STATUS_BIT_OVERFLOW, ((register_a ^ result16) & (result ^ result16) & 0x80));
			register_a = static_cast<uint8_t>(result16);
			UpdateZeroNegative(register_a);
			program_counter += 1;
			break;
		case 0x68:
			register_a = Pop();
			UpdateZeroNegative(register_a);
			break;
		case 0x69:
			operand1 = Read(program_counter + 1);
			result16 = register_a + operand1 + BitCheck(register_p, STATUS_BIT_CARRY);
			/* Check carry/unsigned overflow. */
			SetFlag(STATUS_BIT_CARRY, (result16 & 0x100));
			/* Check signed overflow. */
			SetFlag(STATUS_BIT_OVERFLOW, ((register_a ^ result16) & (operand1 ^ result16) & 0x80));
			register_a = static_cast<uint8_t>(result16);
			UpdateZeroNegative(register_a);
			program_counter += 1;
			break;
		case 0x6A:
			/* ACC */
			/* Check if carry flag is set to preserve value. */
			result = (BitCheck(register_p, STATUS_BIT_CARRY) << 7);
			/* Check if bit 0 is set to preserve value. */
			SetFlag(STATUS_BIT_CARRY, (register_a & 0x01));
			/* Perform shift, add old carry bit. */
			result += (register_a >> 1);
			/* Finally write value. */
			register_a = result;
			UpdateZeroNegative(register_a);
			break;
		case 0x6B:
			illegal_opcode_triggered = true;
			operand1 = Read(program_counter + 1);
			std::cout << "ARR #i unimp\n";
			program_counter += 2;
			break;
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
			SetFlag(STATUS_BIT_CARRY, (result & 0x100));
			/* Check signed overflow. */
			SetFlag(STATUS_BIT_OVERFLOW, ((register_a ^ result) & (Read((operand2 << 8) + operand1) ^ result) & 0x80));
			register_a = static_cast<uint8_t>(result);
			UpdateZeroNegative(register_a);
			program_counter += 2;
			break;
		case 0x6E:
			/* ABS */
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			/* Check if carry flag is set to preserve value. */
			result = BitCheck(register_p, STATUS_BIT_CARRY);
			/* Check if bit 0 is set to preserve value. */
			SetFlag(STATUS_BIT_CARRY, (result & 0x01));
			/* Perform shift, add old carry bit. */
			result += (Read((operand2 << 8) + operand1) >> 1);
			/* Finally write value. */
			Write((operand2 << 8) + operand1, result);
			UpdateZeroNegative(result);
			program_counter += 2;
			break;
		case 0x6F:
			illegal_opcode_triggered = true;
			/* ABS */
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			/* ROR */
			/* Check if carry flag is set to preserve value. */
			result = (BitCheck(register_p, STATUS_BIT_CARRY) << 7);
			/* Check if bit 0 is set to preserve value. */
			SetFlag(STATUS_BIT_CARRY, (Read((operand2 << 8) + operand1) & 0x01));
			/* Perform shift, add old carry bit. */
			result += (Read((operand2 << 8) + operand1) >> 1);
			/* Finally write value. */
			Write((operand2 << 8) + operand1, result);
			UpdateZeroNegative(result);
			/* ADC */
			result16 = register_a + result + BitCheck(register_p, STATUS_BIT_CARRY);
			/* Check carry/unsigned overflow. */
			SetFlag(STATUS_BIT_CARRY, (result16 & 0x100));
			/* Check signed overflow. */
			SetFlag(STATUS_BIT_OVERFLOW, ((register_a ^ result16) & (result ^ result16) & 0x80));
			register_a = static_cast<uint8_t>(result16);
			UpdateZeroNegative(register_a);
			program_counter += 2;
			break;
		case 0x70:
			operand1 = Read(program_counter + 1);
			program_counter++;
			if(BitCheck(register_p, STATUS_BIT_OVERFLOW)) {
				CheckPageCross(program_counter, (program_counter + static_cast<int8_t>(operand1)), 1);
				program_counter += static_cast<int8_t>(operand1);
				cycles += 1;
			}
			break;
		case 0x71:
			/* Fetch address from zero page, add register Y to that address. */
			operand1 = Read(program_counter + 1);
			operand2 = Read(operand1 + 1);
			operand1 = Read(operand1);
			result16 = register_a + Read((operand2 << 8) + operand1 + register_y) + BitCheck(register_p, STATUS_BIT_CARRY);
			/* Check carry/unsigned overflow. */
			SetFlag(STATUS_BIT_CARRY, (result16 & 0x100));
			/* Check signed overflow. */
			SetFlag(STATUS_BIT_OVERFLOW, ((register_a ^ result16) & (Read((operand2 << 8) + operand1 + register_y) ^ result16) & 0x80));
			register_a = static_cast<uint8_t>(result16);
			UpdateZeroNegative(register_a);
			program_counter += 1;
			// TODO: Add 1 to cycle count if crossing page boundary.
			break;
		case 0x72:
			illegal_opcode_triggered = true;
			halted = true;
			/* STP */
			break;
		case 0x73:
			illegal_opcode_triggered = true;
			/* (d),Y */
			operand1 = Read(program_counter + 1);
			operand2 = Read(operand1 + 1);
			operand1 = Read(operand1);
			/* ROR */
			/* Check if carry flag is set to preserve value. */
			result = (BitCheck(register_p, STATUS_BIT_CARRY) << 7);
			/* Check if bit 0 is set to preserve value. */
			SetFlag(STATUS_BIT_CARRY, (Read((operand2 << 8) + operand1 + register_y) & 0x01));
			/* Perform shift, add old carry bit. */
			result += (Read((operand2 << 8) + operand1 + register_y) >> 1);
			/* Finally write value. */
			Write((operand2 << 8) + operand1 + register_y, result);
			UpdateZeroNegative(result);
			/* ADC */
			result16 = register_a + result + BitCheck(register_p, STATUS_BIT_CARRY);
			/* Check carry/unsigned overflow. */
			SetFlag(STATUS_BIT_CARRY, (result16 & 0x100));
			/* Check signed overflow. */
			SetFlag(STATUS_BIT_OVERFLOW, ((register_a ^ result16) & (result ^ result16) & 0x80));
			register_a = static_cast<uint8_t>(result16);
			UpdateZeroNegative(register_a);
			program_counter += 1;
			break;
		case 0x74:
			illegal_opcode_triggered = true;
			/* NOP */
			program_counter += 1;
			break;
		case 0x75:
			/* Zero Page X Index added here to force wrap around instead of carry in the line after. */
			operand1 = Read(program_counter + 1) + register_x;
			operand1 = Read(operand1);
			result16 = register_a + operand1 + BitCheck(register_p, STATUS_BIT_CARRY);
			/* Check carry/unsigned overflow. */
			SetFlag(STATUS_BIT_CARRY, (result16 & 0x100));
			/* Check signed overflow. */
			SetFlag(STATUS_BIT_OVERFLOW, ((register_a ^ result16) & (operand1 ^ result16) & 0x80));
			register_a = static_cast<uint8_t>(result16);
			UpdateZeroNegative(register_a);
			program_counter += 1;
			break;
		case 0x76:
			/* d,X */
			operand1 = Read(program_counter + 1) + register_x;
			operand2 = Read(operand1);
			/* Check if carry flag is set to preserve value. */
			result = (BitCheck(register_p, STATUS_BIT_CARRY) << 7);
			/* Check if bit 0 is set to preserve value. */
			SetFlag(STATUS_BIT_CARRY, (operand2 & 0x01));
			/* Perform shift, add old carry bit. */
			result += (operand2 >> 1);
			/* Finally write value. */
			Write(operand1, result);
			UpdateZeroNegative(result);
			program_counter += 1;
			break;
		case 0x77:
			illegal_opcode_triggered = true;
			/* d,X */
			operand1 = Read(program_counter + 1) + register_x;
			operand2 = Read(operand1);
			/* ROR */
			/* Check if carry flag is set to preserve value. */
			result = (BitCheck(register_p, STATUS_BIT_CARRY) << 7);
			/* Check if bit 0 is set to preserve value. */
			SetFlag(STATUS_BIT_CARRY, (operand2 & 0x01));
			/* Perform shift, add old carry bit. */
			result += (operand2 >> 1);
			/* Finally write value. */
			Write(operand1, result);
			UpdateZeroNegative(result);
			/* ADC */
			result16 = register_a + result + BitCheck(register_p, STATUS_BIT_CARRY);
			/* Check carry/unsigned overflow. */
			SetFlag(STATUS_BIT_CARRY, (result16 & 0x100));
			/* Check signed overflow. */
			SetFlag(STATUS_BIT_OVERFLOW, ((register_a ^ result16) & (result ^ result16) & 0x80));
			register_a = static_cast<uint8_t>(result16);
			UpdateZeroNegative(register_a);
			program_counter += 1;
			break;
		case 0x78:
			SetFlag(STATUS_BIT_INTERRUPT_DISABLE, true);
			break;
		case 0x79:
			/* ABS,Y */
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			result16 = register_a + Read((operand2 << 8) + operand1 + register_y) + BitCheck(register_p, STATUS_BIT_CARRY);
			/* Check carry/unsigned overflow. */
			SetFlag(STATUS_BIT_CARRY, ((result16 & 0x100) == 0));
			/* Check signed overflow. */
			SetFlag(STATUS_BIT_OVERFLOW, ((register_a ^ result16) & (Read((operand2 << 8) + operand1 + register_x) ^ result16) & 0x80));
			register_a = static_cast<uint8_t>(result16);
			UpdateZeroNegative(register_a);
			program_counter += 2;
			// TODO: Add 1 to cycle count if crossing page boundary.
			break;
		case 0x7A:
			illegal_opcode_triggered = true;
			/* NOP */
			break;
		case 0x7B:
			illegal_opcode_triggered = true;
			/* ABS,Y */
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			/* ROR */
			/* Check if carry flag is set to preserve value. */
			result = (BitCheck(register_p, STATUS_BIT_CARRY) << 7);
			/* Check if bit 0 is set to preserve value. */
			SetFlag(STATUS_BIT_CARRY, (Read((operand2 << 8) + operand1 + register_y) & 0x01));
			/* Perform shift, add old carry bit. */
			result += (Read((operand2 << 8) + operand1 + register_y) >> 1);
			/* Finally write value. */
			Write((operand2 << 8) + operand1 + register_y, result);
			UpdateZeroNegative(result);
			/* ADC */
			result16 = register_a + result + BitCheck(register_p, STATUS_BIT_CARRY);
			/* Check carry/unsigned overflow. */
			SetFlag(STATUS_BIT_CARRY, (result16 & 0x100));
			/* Check signed overflow. */
			SetFlag(STATUS_BIT_OVERFLOW, ((register_a ^ result16) & (result ^ result16) & 0x80));
			register_a = static_cast<uint8_t>(result16);
			UpdateZeroNegative(register_a);
			program_counter += 2;
			break;
		case 0x7C:
			illegal_opcode_triggered = true;
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			Read((operand2 << 8) + operand1 + register_x);
			/* NOP */
			// TODO: Add 1 to cycle count if crossing page boundary.
			break;
		case 0x7D:
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			result16 = register_a + Read((operand2 << 8) + operand1 + register_x) + BitCheck(register_p, STATUS_BIT_CARRY);
			/* Check carry/unsigned overflow. */
			SetFlag(STATUS_BIT_CARRY, ((result16 & 0x100) == 0));
			/* Check signed overflow. */
			SetFlag(STATUS_BIT_OVERFLOW, ((register_a ^ result16) & (~Read((operand2 << 8) + operand1 + register_x) ^ result16) & 0x80));
			register_a = static_cast<uint8_t>(result16);
			UpdateZeroNegative(register_a);
			program_counter += 2;
			// TODO: Add 1 to cycle count if crossing page boundary.
			break;
		case 0x7E:
			/* ABS,X */
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			/* Check if carry flag is set to preserve value. */
			result = BitCheck(register_p, STATUS_BIT_CARRY);
			/* Check if bit 0 is set to preserve value. */
			SetFlag(STATUS_BIT_CARRY, (result & 0x01));
			/* Perform shift, add old carry bit. */
			result += (Read((operand2 << 8) + operand1 + register_x) >> 1);
			/* Finally write value. */
			Write((operand2 << 8) + operand1 + register_x, result);
			UpdateZeroNegative(result);
			program_counter += 2;
			break;
		case 0x7F:
			illegal_opcode_triggered = true;
			/* ABS,X */
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			/* ROR */
			/* Check if carry flag is set to preserve value. */
			result = (BitCheck(register_p, STATUS_BIT_CARRY) << 7);
			/* Check if bit 0 is set to preserve value. */
			SetFlag(STATUS_BIT_CARRY, (Read((operand2 << 8) + operand1 + register_x) & 0x01));
			/* Perform shift, add old carry bit. */
			result += (Read((operand2 << 8) + operand1 + register_x) >> 1);
			/* Finally write value. */
			Write((operand2 << 8) + operand1 + register_x, result);
			UpdateZeroNegative(result);
			/* ADC */
			result16 = register_a + result + BitCheck(register_p, STATUS_BIT_CARRY);
			/* Check carry/unsigned overflow. */
			SetFlag(STATUS_BIT_CARRY, (result16 & 0x100));
			/* Check signed overflow. */
			SetFlag(STATUS_BIT_OVERFLOW, ((register_a ^ result16) & (result ^ result16) & 0x80));
			register_a = static_cast<uint8_t>(result16);
			UpdateZeroNegative(register_a);
			program_counter += 2;
			break;
		case 0x80:
			illegal_opcode_triggered = true;
			Read(program_counter + 1);
			/* NOP */
			break;
		case 0x81:
			/* (d,X) */
			operand1 = Read(program_counter + 1);
			operand2 = Read(static_cast<uint8_t>(operand1 + register_x + 1));
			operand1 = Read(static_cast<uint8_t>(operand1 + register_x));
			Write(Read((operand2 << 8) + operand1), register_a);
			program_counter += 1;
			break;
		case 0x82:
			illegal_opcode_triggered = true;
			Read(program_counter + 1);
			/* NOP */
			break;
		case 0x83:
			illegal_opcode_triggered = true;
			/* (d,X) */
			operand1 = Read(program_counter + 1);
			operand2 = Read(static_cast<uint8_t>(operand1 + register_x + 1));
			operand1 = Read(static_cast<uint8_t>(operand1 + register_x));
			/* AND */
			result = register_a & register_x;
			/* STX */
			Write(Read((operand2 << 8) + operand1), result);
			program_counter += 1;
			break;
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
		case 0x87:
			illegal_opcode_triggered = true;
			/* d */
			operand1 = Read(program_counter + 1);
			/* AND */
			result = register_a & register_x;
			/* STX */
			Write(operand1, result);
			program_counter += 1;
			break;
		case 0x88:
			register_y--;
			UpdateZeroNegative(register_y);
			break;
		case 0x89:
			illegal_opcode_triggered = true;
			Read(program_counter + 1);
			/* NOP */
			break;
		case 0x8A:
			register_a = register_x;
			UpdateZeroNegative(register_a);
			break;
		case 0x8B:
			illegal_opcode_triggered = true;
			operand1 = Read(program_counter + 1);
			halted = true;
			program_counter += 1;
			/* XAA */
			break;
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
		case 0x8F:
			illegal_opcode_triggered = true;
			/* Abs */
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			/* AND */
			result = register_a & register_x;
			/* STX */
			Write(((operand2 << 8) + operand1), result);
			program_counter += 2;
			break;
		case 0x90:
			operand1 = Read(program_counter + 1);
			program_counter++;
			if(!BitCheck(register_p, STATUS_BIT_CARRY)) {
				CheckPageCross(program_counter, (program_counter + static_cast<int8_t>(operand1)), 1);
				program_counter += static_cast<int8_t>(operand1);
				cycles += 1;
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
		case 0x92:
			illegal_opcode_triggered = true;
			halted = true;
			/* STP */
			break;
		case 0x93:
			illegal_opcode_triggered = true;
			operand1 = Read(program_counter + 1);
			std::cout << "AHX (d),y unimp\n";
			program_counter += 1;
			break;
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
			Write(operand1, register_x);
			program_counter += 1;
			break;
		case 0x97:
			illegal_opcode_triggered = true;
			/* d,Y */
			operand1 = Read(program_counter + 1) + register_y;
			/* AND */
			result = register_a & register_x;
			/* STX */
			Write(operand1, result);
			program_counter += 1;
			break;
		case 0x98:
			register_a = register_y;
			UpdateZeroNegative(register_a);
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
		case 0x9B:
			illegal_opcode_triggered = true;
			break;
		case 0x9C:
			illegal_opcode_triggered = true;
			break;
		case 0x9D:
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			Write(((operand2 << 8) + operand1 + register_x), register_a);
			program_counter += 2;
			break;
		case 0x9E:
			illegal_opcode_triggered = true;
			break;
		case 0x9F:
			illegal_opcode_triggered = true;
			break;
		case 0xA0:
			operand1 = Read(program_counter + 1);
			register_y = operand1;
			UpdateZeroNegative(register_y);
			program_counter += 1;
			break;
		case 0xA1:
			/* (d,X) */
			operand1 = Read(program_counter + 1);
			operand2 = Read(static_cast<uint8_t>(operand1 + register_x + 1));
			operand1 = Read(static_cast<uint8_t>(operand1 + register_x));
			register_a = Read((operand2 << 8) + operand1);
			UpdateZeroNegative(register_a);
			program_counter += 1;
			break;
		case 0xA2:
			operand1 = Read(program_counter + 1);
			register_x = operand1;
			UpdateZeroNegative(register_x);
			program_counter += 1;
			break;
		case 0xA3:
			illegal_opcode_triggered = true;
			/* (d,X) */
			operand1 = Read(program_counter + 1);
			operand2 = Read(static_cast<uint8_t>(operand1 + register_x + 1));
			operand1 = Read(static_cast<uint8_t>(operand1 + register_x));
			/* LDA */
			register_a = Read((operand2 << 8) + operand1);
			UpdateZeroNegative(register_a);
			/* TAX */
			register_x = register_a;
			UpdateZeroNegative(register_x);
			program_counter += 1;
			break;
		case 0xA4:
			operand1 = Read(program_counter + 1);
			register_y = Read(operand1);
			UpdateZeroNegative(register_y);
			program_counter += 1;
			break;
		case 0xA5:
			operand1 = Read(program_counter + 1);
			register_a = Read(operand1);
			UpdateZeroNegative(register_a);
			program_counter += 1;
			break;
		case 0xA6:
			operand1 = Read(program_counter + 1);
			register_x = Read(operand1);
			UpdateZeroNegative(register_x);
			program_counter += 1;
			break;
		case 0xA7:
			illegal_opcode_triggered = true;
			/* d */
			operand1 = Read(program_counter + 1);
			/* LDA */
			register_a = Read(operand1);
			UpdateZeroNegative(register_a);
			/* TAX */
			register_x = register_a;
			UpdateZeroNegative(register_x);
			program_counter += 1;
			break;
		case 0xA8:
			register_y = register_a;
			UpdateZeroNegative(register_y);
			break;
		case 0xA9:
			register_a = Read(program_counter + 1);
			UpdateZeroNegative(register_a);
			program_counter += 1;
			break;
		case 0xAA:
			register_x = register_a;
			UpdateZeroNegative(register_x);
			break;
		case 0xAB:
			illegal_opcode_triggered = true;
			/* IMM */
			operand1 = Read(program_counter + 1);
			/* LDA */
			register_a = operand1;
			UpdateZeroNegative(register_a);
			/* TAX */
			register_x = register_a;
			UpdateZeroNegative(register_x);
			program_counter += 1;
			break;
		case 0xAC:
			/* ABS */
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			register_y = Read((operand2 << 8) + operand1);
			UpdateZeroNegative(register_y);
			program_counter += 2;
			break;
		case 0xAD:
			/* ABS */
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			register_a = Read((operand2 << 8) + operand1);
			UpdateZeroNegative(register_a);
			program_counter += 2;
			break;
		case 0xAE:
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			register_x = Read((operand2 << 8) + operand1);
			UpdateZeroNegative(register_x);
			program_counter += 2;
			break;
		case 0xAF:
			illegal_opcode_triggered = true;
			/* ABS */
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			/* LDA */
			register_a = Read((operand2 << 8) + operand1);
			UpdateZeroNegative(register_a);
			/* TAX */
			register_x = register_a;
			UpdateZeroNegative(register_x);
			program_counter += 2;
			break;
		case 0xB0:
			operand1 = Read(program_counter + 1);
			program_counter++;
			if(BitCheck(register_p, STATUS_BIT_CARRY)) {
				CheckPageCross(program_counter, (program_counter + static_cast<int8_t>(operand1)), 1);
				program_counter += static_cast<int8_t>(operand1);
				cycles += 1;
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
			UpdateZeroNegative(register_a);
			program_counter += 1;
			// TODO: Add 1 to cycle count if crossing page boundary.
			break;
		case 0xB2:
			illegal_opcode_triggered = true;
			halted = true;
			/* STP */
			break;
		case 0xB3:
			illegal_opcode_triggered = true;
			/* (d),Y */
			operand1 = Read(program_counter + 1);
			operand2 = Read(operand1 + 1);
			operand1 = Read(operand1);
			/* LDA */
			register_a = Read((operand2 << 8) + operand1 + register_y);
			UpdateZeroNegative(register_a);
			/* TAX */
			register_x = register_a;
			UpdateZeroNegative(register_x);
			program_counter += 1;
			break;
		case 0xB4:
			/* d,X */
			operand1 = Read(program_counter + 1) + register_x;
			register_y = Read(operand1);
			UpdateZeroNegative(register_y);
			program_counter += 1;
			break;
		case 0xB5:
			/* d,X */
			operand1 = Read(program_counter + 1) + register_x;
			register_a = Read(operand1);
			UpdateZeroNegative(register_a);
			program_counter += 1;
			break;
		case 0xB6:
			/* d,Y */
			operand1 = Read(program_counter + 1) + register_y;
			register_x = Read(operand1);
			UpdateZeroNegative(register_x);
			program_counter += 1;
			break;
		case 0xB7:
			illegal_opcode_triggered = true;
			/* d,Y */
			operand1 = Read(program_counter + 1) + register_y;
			/* LDA */
			register_a = operand1;
			UpdateZeroNegative(register_a);
			/* TAX */
			register_x = register_a;
			UpdateZeroNegative(register_x);
			program_counter += 1;
			break;
		case 0xB8:
			SetFlag(STATUS_BIT_OVERFLOW, false);
			break;
		case 0xB9:
			/* ABS,Y */
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			register_a = Read((operand2 << 8) + operand1 + register_y);
			UpdateZeroNegative(register_a);
			program_counter += 2;
			// TODO: Add 1 to cycle count if crossing page boundary.
			break;
		case 0xBA:
			register_x = register_s;
			UpdateZeroNegative(register_x);
			break;
		case 0xBB:
			illegal_opcode_triggered = true;
			/* ABS,Y */
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			std::cout << "LAS a,y unimp\n";
			program_counter += 2;
			break;
		case 0xBC:
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			register_y = Read((operand2 << 8) + operand1 + register_x);
			UpdateZeroNegative(register_y);
			program_counter += 2;
			// TODO: Add 1 to cycle count if crossing page boundary.
			break;
		case 0xBD:
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			register_a = Read((operand2 << 8) + operand1 + register_x);
			UpdateZeroNegative(register_a);
			program_counter += 2;
			// TODO: Add 1 to cycle count if crossing page boundary.
			break;
		case 0xBE:
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			register_x = Read((operand2 << 8) + operand1 + register_y);
			UpdateZeroNegative(register_x);
			program_counter += 2;
			// TODO: Add 1 to cycle count if crossing page boundary.
			break;
		case 0xBF:
			illegal_opcode_triggered = true;
			/* Abs */
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			/* LDA */
			register_a = Read((operand2 << 8) + operand1 + register_y);
			UpdateZeroNegative(register_a);
			/* TAX */
			register_x = register_a;
			UpdateZeroNegative(register_x);
			program_counter += 2;
			break;
		case 0xC0:
			operand1 = Read(program_counter + 1);
			result = (register_y - operand1);
			SetFlag(STATUS_BIT_CARRY,    (register_y >= operand1));
			SetFlag(STATUS_BIT_ZERO,     (register_y == operand1));
			SetFlag(STATUS_BIT_NEGATIVE, (result >= 0x80));
			program_counter += 1;
			break;
		case 0xC1:
			/* (d,X) */
			operand1 = Read(program_counter + 1);
			operand2 = Read(static_cast<uint8_t>(operand1 + register_x + 1));
			operand1 = Read(static_cast<uint8_t>(operand1 + register_x));
			result = (register_a - Read((operand2 << 8) + operand1));
			SetFlag(STATUS_BIT_CARRY,    (register_a >= Read((operand2 << 8) + operand1)));
			SetFlag(STATUS_BIT_ZERO,     (register_a == Read((operand2 << 8) + operand1)));
			SetFlag(STATUS_BIT_NEGATIVE, (result >= 0x80));
			program_counter += 1;
			break;
		case 0xC2:
			illegal_opcode_triggered = true;
			Read(program_counter + 1);
			/* NOP */
			break;
		case 0xC3:
			illegal_opcode_triggered = true;
			/* (d,X) */
			operand1 = Read(program_counter + 1);
			operand2 = Read(static_cast<uint8_t>(operand1 + register_x + 1));
			operand1 = Read(static_cast<uint8_t>(operand1 + register_x));
			/* DEC */
			result = Read((operand2 << 8) + operand1);
			result--;
			Write(Read((operand2 << 8) + operand1), result);
			UpdateZeroNegative(result);
			/* CMP */
			result = (register_a - Read((operand2 << 8) + operand1));
			SetFlag(STATUS_BIT_CARRY,    (register_a >= Read((operand2 << 8) + operand1)));
			SetFlag(STATUS_BIT_ZERO,     (register_a == Read((operand2 << 8) + operand1)));
			SetFlag(STATUS_BIT_NEGATIVE, (result >= 0x80));
			program_counter += 1;
			break;
		case 0xC4:
			operand1 = Read(program_counter + 1);
			operand2 = Read(operand1);
			result = (register_y - operand2);
			SetFlag(STATUS_BIT_CARRY,    (register_y >= operand2));
			SetFlag(STATUS_BIT_ZERO,     (register_y == operand2));
			SetFlag(STATUS_BIT_NEGATIVE, (result >= 0x80));
			program_counter += 1;
			break;
		case 0xC5:
			operand1 = Read(program_counter + 1);
			operand2 = Read(operand1);
			result = (register_a - operand2);
			SetFlag(STATUS_BIT_CARRY,    (register_a >= operand2));
			SetFlag(STATUS_BIT_ZERO,     (register_a == operand2));
			SetFlag(STATUS_BIT_NEGATIVE, (result >= 0x80));
			program_counter += 1;
			break;
		case 0xC6:
			operand1 = Read(program_counter + 1);
			operand2 = Read(operand1);
			operand2--;
			Write(operand1, operand2);
			UpdateZeroNegative(operand2);
			program_counter += 1;
			break;
		case 0xC7:
			illegal_opcode_triggered = true;
			/* d */
			operand1 = Read(program_counter + 1);
			/* DEC */
			result = Read(operand1);
			result--;
			Write(operand1, result);
			UpdateZeroNegative(result);
			/* CMP */
			result = (register_a - Read(operand1));
			SetFlag(STATUS_BIT_CARRY,    (register_a >= Read(operand1)));
			SetFlag(STATUS_BIT_ZERO,     (register_a == Read(operand1)));
			SetFlag(STATUS_BIT_NEGATIVE, (result >= 0x80));
			program_counter += 1;
			break;
		case 0xC8:
			register_y++;
			UpdateZeroNegative(register_y);
			break;
		case 0xC9:
			operand1 = Read(program_counter + 1);
			result = (register_a - operand1);
			SetFlag(STATUS_BIT_CARRY,    (register_a >= operand1));
			SetFlag(STATUS_BIT_ZERO,     (register_a == operand1));
			SetFlag(STATUS_BIT_NEGATIVE, (result >= 0x80));
			program_counter += 1;
			break;
		case 0xCA:
			register_x--;
			UpdateZeroNegative(register_x);
			break;
		case 0xCB:
			illegal_opcode_triggered = true;
			/* IMM */
			operand1 = Read(program_counter + 1);
			std::cout << "AXS #i unimp\n";
			program_counter += 1;
			break;
		case 0xCC:
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			result = Read((operand2 << 8) + operand1);
			SetFlag(STATUS_BIT_CARRY,    (register_y >= result));
			SetFlag(STATUS_BIT_ZERO,     (register_y == result));
			result = (register_y - result);
			SetFlag(STATUS_BIT_NEGATIVE, (result >= 0x80));
			program_counter += 2;
			break;
		case 0xCD:
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			result = Read((operand2 << 8) + operand1);
			SetFlag(STATUS_BIT_CARRY,    (register_a >= result));
			SetFlag(STATUS_BIT_ZERO,     (register_a == result));
			result = (register_a - Read((operand2 << 8) + operand1));
			SetFlag(STATUS_BIT_NEGATIVE, (result >= 0x80));
			program_counter += 2;
			break;
		case 0xCE:
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			result = Read((operand2 << 8) + operand1);
			result--;
			Write((operand2 << 8) + operand1, result);
			UpdateZeroNegative(result);
			program_counter += 2;
			break;
		case 0xCF:
			illegal_opcode_triggered = true;
			/* ABS */
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			/* DEC */
			result = Read((operand2 << 8) + operand1);
			result--;
			Write((operand2 << 8) + operand1, result);
			UpdateZeroNegative(result);
			/* CMP */
			result = (register_a - Read((operand2 << 8) + operand1));
			SetFlag(STATUS_BIT_CARRY,    (register_a >= Read((operand2 << 8) + operand1)));
			SetFlag(STATUS_BIT_ZERO,     (register_a == Read((operand2 << 8) + operand1)));
			SetFlag(STATUS_BIT_NEGATIVE, (result >= 0x80));
			program_counter += 2;
			break;
		case 0xD0:
			operand1 = Read(program_counter + 1);
			program_counter++;
			if(BitCheck(register_p, STATUS_BIT_ZERO) == 0) {
				CheckPageCross(program_counter, (program_counter + static_cast<int8_t>(operand1)), 1);
				program_counter += static_cast<int8_t>(operand1);
				cycles += 1;
			}
			break;
		case 0xD1:
			/* Fetch address from zero page, add register Y to that address. */
			operand1 = Read(program_counter + 1);
			operand2 = Read(operand1 + 1);
			operand1 = Read(operand1);
			result = (register_a - Read((operand2) << 8) + operand1 + register_y);
			SetFlag(STATUS_BIT_CARRY,    (register_a >= Read(((operand2) << 8) + operand1 + register_y)));
			SetFlag(STATUS_BIT_ZERO,     (register_a == Read(((operand2) << 8) + operand1 + register_y)));
			SetFlag(STATUS_BIT_NEGATIVE, (result >= 0x80));
			// TODO: Add 1 to cycle count if crossing page boundary.
			program_counter += 1;
			break;
		case 0xD2:
			illegal_opcode_triggered = true;
			halted = true;
			/* STP */
			break;
		case 0xD3:
			illegal_opcode_triggered = true;
			/* d,(Y) */
			operand1 = Read(program_counter + 1);
			operand2 = Read(operand1 + 1);
			operand1 = Read(operand1);
			/* DEC */
			result = Read((operand2 << 8) + operand1 + register_y);
			result--;
			Write(Read((operand2 << 8) + operand1 + register_y), result);
			UpdateZeroNegative(result);
			/* CMP */
			result = (register_a - Read((operand2) << 8) + operand1 + register_y);
			SetFlag(STATUS_BIT_CARRY,    (register_a >= Read(((operand2) << 8) + operand1 + register_y)));
			SetFlag(STATUS_BIT_ZERO,     (register_a == Read(((operand2) << 8) + operand1 + register_y)));
			SetFlag(STATUS_BIT_NEGATIVE, (result >= 0x80));
			// TODO: Add 1 to cycle count if crossing page boundary.
			program_counter += 1;
			break;
		case 0xD4:
			illegal_opcode_triggered = true;
			Read(program_counter + 1);
			/* NOP */
			program_counter += 1;
			break;
		case 0xD5:
			/* Zero Page X Index added here to force wrap around instead of carry in the line after. */
			operand1 = Read(program_counter + 1) + register_x;
			operand2 = Read(operand1);
			result = (register_a - operand2);
			SetFlag(STATUS_BIT_CARRY,    (register_a >= operand2));
			SetFlag(STATUS_BIT_ZERO,     (register_a == operand2));
			SetFlag(STATUS_BIT_NEGATIVE, (result >= 0x80));
			program_counter += 1;
			break;
		case 0xD6:
			/* Zero Page X Index added here to force wrap around instead of carry in the line after. */
			operand1 = Read(program_counter + 1) + register_x;
			operand2 = Read(operand1);
			operand2--;
			Write(operand1, operand2);
			UpdateZeroNegative(operand2);
			program_counter += 1;
			break;
		case 0xD7:
			illegal_opcode_triggered = true;
			/* d,X */
			operand1 = Read(program_counter + 1) + register_x;
			/* DEC */
			result = Read(operand1);
			result--;
			Write(operand1, result);
			UpdateZeroNegative(result);
			/* CMP */
			result = (register_a - Read(operand1));
			SetFlag(STATUS_BIT_CARRY,    (register_a >= Read(operand1)));
			SetFlag(STATUS_BIT_ZERO,     (register_a == Read(operand1)));
			SetFlag(STATUS_BIT_NEGATIVE, (result >= 0x80));
			program_counter += 1;
			break;
		case 0xD8:
			BitClear(register_p, STATUS_BIT_DECIMAL);
			break;
		case 0xD9:
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			result = Read((operand2 << 8) + operand1 + register_y);
			SetFlag(STATUS_BIT_CARRY,    (register_a >= result));
			SetFlag(STATUS_BIT_ZERO,     (register_a == result));
			result = (register_a - Read((operand2 << 8) + operand1 + register_y));
			SetFlag(STATUS_BIT_NEGATIVE, (result >= 0x80));
			program_counter += 2;
			// TODO: Add 1 to cycle count if crossing page boundary.
			break;
		case 0xDA:
			illegal_opcode_triggered = true;
			/* NOP */
			break;
		case 0xDB:
			illegal_opcode_triggered = true;
			/* ABS,Y */
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			/* DEC */
			result = Read((operand2 << 8) + operand1 + register_y);
			result--;
			Write((operand2 << 8) + operand1 + register_y, result);
			UpdateZeroNegative(result);
			/* CMP */
			result = (register_a - Read((operand2 << 8) + operand1 + register_y));
			SetFlag(STATUS_BIT_CARRY,    (register_a >= Read((operand2 << 8) + operand1 + register_y)));
			SetFlag(STATUS_BIT_ZERO,     (register_a == Read((operand2 << 8) + operand1 + register_y)));
			SetFlag(STATUS_BIT_NEGATIVE, (result >= 0x80));
			program_counter += 2;
			break;
		case 0xDC:
			illegal_opcode_triggered = true;
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			Read((operand2 << 8) + operand1 + register_x);
			/* NOP */
			// TODO: Add 1 to cycle count if crossing page boundary.
			break;
		case 0xDD:
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			result = Read((operand2 << 8) + operand1 + register_x);
			SetFlag(STATUS_BIT_CARRY,    (register_a >= result));
			SetFlag(STATUS_BIT_ZERO,     (register_a == result));
			result = (register_a - Read((operand2 << 8) + operand1 + register_x));
			SetFlag(STATUS_BIT_NEGATIVE, (result >= 0x80));
			program_counter += 2;
			// TODO: Add 1 to cycle count if crossing page boundary.
			break;
		case 0xDE:
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			result = Read((operand2 << 8) + operand1 + register_x);
			result--;
			Write(((operand2 << 8) + operand1 + register_x), result);
			UpdateZeroNegative(result);
			program_counter += 2;
			break;
		case 0xDF:
			illegal_opcode_triggered = true;
			/* ABS,X */
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			/* DEC */
			result = Read((operand2 << 8) + operand1 + register_x);
			result--;
			Write((operand2 << 8) + operand1 + register_x, result);
			UpdateZeroNegative(result);
			/* CMP */
			result = (register_a - Read((operand2 << 8) + operand1 + register_x));
			SetFlag(STATUS_BIT_CARRY,    (register_a >= Read((operand2 << 8) + operand1 + register_x)));
			SetFlag(STATUS_BIT_ZERO,     (register_a == Read((operand2 << 8) + operand1 + register_x)));
			SetFlag(STATUS_BIT_NEGATIVE, (result >= 0x80));
			program_counter += 2;
			break;
		case 0xE0:
			operand1 = Read(program_counter + 1);
			result = (register_x - operand1);
			SetFlag(STATUS_BIT_CARRY,    (register_x >= operand1));
			SetFlag(STATUS_BIT_ZERO,     (register_x == operand1));
			SetFlag(STATUS_BIT_NEGATIVE, (result >= 0x80));
			program_counter += 1;
			break;
		case 0xE1:
			/* (d,X) */
			operand1 = Read(program_counter + 1);
			operand2 = Read(static_cast<uint8_t>(operand1 + register_x + 1));
			operand1 = Read(static_cast<uint8_t>(operand1 + register_x));
			result16 = (register_a - (Read((operand2 << 8) + operand1)) - !BitCheck(register_p, STATUS_BIT_CARRY));
			/* Check carry/unsigned overflow. */
			SetFlag(STATUS_BIT_CARRY, ((result16 & 0x100) == 0));
			/* Check signed overflow. */
			SetFlag(STATUS_BIT_OVERFLOW, ((register_a ^ result16) & (~(Read((operand2 << 8) + operand1)) ^ result16) & 0x80));
			register_a = static_cast<uint8_t>(result16);
			UpdateZeroNegative(register_a);
			program_counter += 1;
			break;
		case 0xE2:
			illegal_opcode_triggered = true;
			Read(program_counter + 1);
			/* NOP */
			break;
		case 0xE3:
			illegal_opcode_triggered = true;
			/* (d,X) */
			operand1 = Read(program_counter + 1);
			operand2 = Read(static_cast<uint8_t>(operand1 + register_x + 1));
			operand1 = Read(static_cast<uint8_t>(operand1 + register_x));
			/* INC */
			result = Read((operand2 << 8) + operand1);
			result++;
			Write(Read((operand2 << 8) + operand1), result);
			UpdateZeroNegative(result);
			/* SBC */
			result16 = (register_a - (Read((operand2 << 8) + operand1)) - !BitCheck(register_p, STATUS_BIT_CARRY));
			/* Check carry/unsigned overflow. */
			SetFlag(STATUS_BIT_CARRY, ((result16 & 0x100) == 0));
			/* Check signed overflow. */
			SetFlag(STATUS_BIT_OVERFLOW, ((register_a ^ result16) & (~(Read((operand2 << 8) + operand1)) ^ result16) & 0x80));
			register_a = static_cast<uint8_t>(result16);
			UpdateZeroNegative(register_a);
			program_counter += 1;
			break;
		case 0xE4:
			operand1 = Read(program_counter + 1);
			operand1 = Read(operand1);
			result = (register_x - operand1);
			SetFlag(STATUS_BIT_CARRY,    (register_x >= operand1));
			SetFlag(STATUS_BIT_ZERO,     (register_x == operand1));
			SetFlag(STATUS_BIT_NEGATIVE, (result >= 0x80));
			program_counter += 1;
			break;
		case 0xE5:
			operand1 = Read(program_counter + 1);
			operand1 = Read(operand1);
			result16 = (register_a - operand1 - !BitCheck(register_p, STATUS_BIT_CARRY));
			/* Check carry/unsigned overflow. */
			SetFlag(STATUS_BIT_CARRY, ((result16 & 0x100) == 0));
			/* Check signed overflow. */
			SetFlag(STATUS_BIT_OVERFLOW, ((register_a ^ result16) & (~operand1 ^ result16) & 0x80));
			register_a = static_cast<uint8_t>(result16);
			UpdateZeroNegative(register_a);
			program_counter += 1;
			break;
		case 0xE6:
			operand1 = Read(program_counter + 1);
			operand2 = Read(operand1);
			operand2++;
			Write(operand1, operand2);
			UpdateZeroNegative(operand2);
			program_counter += 1;
			break;
		case 0xE7:
			illegal_opcode_triggered = true;
			/* d */
			operand1 = Read(program_counter + 1);
			/* INC */
			result = Read(operand1);
			result++;
			Write(operand1, result);
			UpdateZeroNegative(result);
			/* SBC */
			result16 = (register_a - Read(operand1) - !BitCheck(register_p, STATUS_BIT_CARRY));
			/* Check carry/unsigned overflow. */
			SetFlag(STATUS_BIT_CARRY, ((result16 & 0x100) == 0));
			/* Check signed overflow. */
			SetFlag(STATUS_BIT_OVERFLOW, ((register_a ^ result16) & (~(Read(operand1)) ^ result16) & 0x80));
			register_a = static_cast<uint8_t>(result16);
			UpdateZeroNegative(register_a);
			program_counter += 1;
			break;
		case 0xE8:
			register_x++;
			UpdateZeroNegative(register_x);
			break;
		case 0xE9:
			operand1 = Read(program_counter + 1);
			result16 = (register_a - operand1 - !BitCheck(register_p, STATUS_BIT_CARRY));
			/* Check carry/unsigned overflow. */
			SetFlag(STATUS_BIT_CARRY, ((result16 & 0x100) == 0));
			/* Check signed overflow. */
			SetFlag(STATUS_BIT_OVERFLOW, ((register_a ^ result16) & (~operand1 ^ result16) & 0x80));
			register_a = static_cast<uint8_t>(result16);
			UpdateZeroNegative(register_a);
			program_counter += 1;
			break;
		case 0xEA:
			/* NOP */
			break;
		case 0xEB:
			illegal_opcode_triggered = true;
			/* IMM */
			operand1 = Read(program_counter + 1);
			result16 = register_a + operand1 + BitCheck(register_p, STATUS_BIT_CARRY);
			/* Check carry/unsigned overflow. */
			SetFlag(STATUS_BIT_CARRY, ((result16 & 0x100) == 0));
			/* Check signed overflow. */
			SetFlag(STATUS_BIT_OVERFLOW, ((register_a ^ result16) & (operand1 ^ result16) & 0x80));
			register_a = static_cast<uint8_t>(result16);
			UpdateZeroNegative(register_a);
			program_counter += 1;
			break;
		case 0xEC:
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			result = Read((operand2 << 8) + operand1);
			SetFlag(STATUS_BIT_CARRY,    (register_x >= result));
			SetFlag(STATUS_BIT_ZERO,     (register_x == result));
			result = (register_x - result);
			SetFlag(STATUS_BIT_NEGATIVE, (result >= 0x80));
			program_counter += 2;
			break;
		case 0xED:
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			result16 = (register_a - (Read((operand2 << 8) + operand1)) - !BitCheck(register_p, STATUS_BIT_CARRY));
			/* Check carry/unsigned overflow. */
			SetFlag(STATUS_BIT_CARRY, ((result16 & 0x100) == 0));
			/* Check signed overflow. */
			SetFlag(STATUS_BIT_OVERFLOW, ((register_a ^ result16) & (~(Read((operand2 << 8) + operand1)) ^ result16) & 0x80));
			register_a = static_cast<uint8_t>(result16);
			UpdateZeroNegative(register_a);
			program_counter += 2;
			break;
		case 0xEE:
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			result = Read((operand2 << 8) + operand1);
			result++;
			Write((operand2 << 8) + operand1, result);
			UpdateZeroNegative(result);
			program_counter += 2;
			break;
		case 0xEF:
			illegal_opcode_triggered = true;
			/* ABS */
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			/* INC */
			result = Read((operand2 << 8) + operand1);
			result++;
			Write((operand2 << 8) + operand1, result);
			UpdateZeroNegative(result);
			/* SBC */
			result16 = (register_a - Read((operand2 << 8) + operand1) - !BitCheck(register_p, STATUS_BIT_CARRY));
			/* Check carry/unsigned overflow. */
			SetFlag(STATUS_BIT_CARRY, ((result16 & 0x100) == 0));
			/* Check signed overflow. */
			SetFlag(STATUS_BIT_OVERFLOW, ((register_a ^ result16) & (~(Read((operand2 << 8) + operand1)) ^ result16) & 0x80));
			register_a = static_cast<uint8_t>(result16);
			UpdateZeroNegative(register_a);
			program_counter += 2;
			break;
		case 0xF0:
			operand1 = Read(program_counter + 1);
			program_counter++;
			if(BitCheck(register_p, STATUS_BIT_ZERO) == 1) {
				CheckPageCross(program_counter, (program_counter + static_cast<int8_t>(operand1)), 1);
				program_counter += static_cast<int8_t>(operand1);
				cycles += 1;
			}
			break;
		case 0xF1:
			/* Fetch address from zero page, add register Y to that address. */
			operand1 = Read(program_counter + 1);
			operand2 = Read(operand1 + 1);
			operand1 = Read(operand1);
			result16 = (register_a - Read((operand2 << 8) + operand1 + register_y) - !BitCheck(register_p, STATUS_BIT_CARRY));
			/* Check carry/unsigned overflow. */
			SetFlag(STATUS_BIT_CARRY, ((result16 & 0x100) == 0));
			/* Check signed overflow. */
			SetFlag(STATUS_BIT_OVERFLOW, ((register_a ^ result16) & (~Read((operand2 << 8) + operand1 + register_y) ^ result16) & 0x80));
			register_a = static_cast<uint8_t>(result16);
			UpdateZeroNegative(register_a);
			// TODO: Add 1 to cycle count if crossing page boundary.
			program_counter += 1;
			break;
		case 0xF2:
			illegal_opcode_triggered = true;
			halted = true;
			/* STP */
			break;
		case 0xF3:
			illegal_opcode_triggered = true;
			/* (d),Y */
			operand1 = Read(program_counter + 1);
			operand2 = Read(operand1 + 1);
			operand1 = Read(operand1);
			/* INC */
			result = Read((operand2 << 8) + operand1 + register_y);
			result++;
			Write((operand2 << 8) + operand1 + register_y, result);
			UpdateZeroNegative(result);
			/* SBC */
			result16 = (register_a - Read((operand2 << 8) + operand1 + register_y) - !BitCheck(register_p, STATUS_BIT_CARRY));
			/* Check carry/unsigned overflow. */
			SetFlag(STATUS_BIT_CARRY, ((result16 & 0x100) == 0));
			/* Check signed overflow. */
			SetFlag(STATUS_BIT_OVERFLOW, ((register_a ^ result16) & (~(result) ^ result16) & 0x80));
			register_a = static_cast<uint8_t>(result16);
			UpdateZeroNegative(register_a);
			program_counter += 1;
			break;
		case 0xF4:
			illegal_opcode_triggered = true;
			Read(program_counter + 1);
			/* NOP */
			program_counter += 1;
			break;
		case 0xF5:
			/* Zero Page X Index added here to force wrap around instead of carry in the line after. */
			operand1 = Read(program_counter + 1) + register_x;
			operand1 = Read(operand1);
			result16 = (register_a - operand1 - !BitCheck(register_p, STATUS_BIT_CARRY));
			/* Check carry/unsigned overflow. */
			SetFlag(STATUS_BIT_CARRY, ((result16 & 0x100) == 0));
			/* Check signed overflow. */
			SetFlag(STATUS_BIT_OVERFLOW, ((register_a ^ result16) & (~operand1 ^ result16) & 0x80));
			register_a = static_cast<uint8_t>(result16);
			UpdateZeroNegative(register_a);
			program_counter += 1;
			break;
		case 0xF6:
			/* Zero Page X Index added here to force wrap around instead of carry in the line after. */
			operand1 = Read(program_counter + 1) + register_x;
			operand2 = Read(operand1);
			operand2++;
			Write(operand1, operand2);
			UpdateZeroNegative(operand2);
			program_counter += 1;
			break;
		case 0xF7:
			illegal_opcode_triggered = true;
			/* d,X */
			operand1 = Read(program_counter + 1) + register_x;
			/* INC */
			result = Read(operand1);
			result++;
			Write(operand1, result);
			UpdateZeroNegative(result);
			/* SBC */
			result16 = (register_a - Read(operand1) - !BitCheck(register_p, STATUS_BIT_CARRY));
			/* Check carry/unsigned overflow. */
			SetFlag(STATUS_BIT_CARRY, ((result16 & 0x100) == 0));
			/* Check signed overflow. */
			SetFlag(STATUS_BIT_OVERFLOW, ((register_a ^ result16) & (~(Read(operand1)) ^ result16) & 0x80));
			register_a = static_cast<uint8_t>(result16);
			UpdateZeroNegative(register_a);
			program_counter += 1;
			break;
		case 0xF8:
			BitSet(register_p, STATUS_BIT_DECIMAL);
			break;
		case 0xF9:
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			result16 = (register_a - Read((operand2 << 8) + operand1 + register_y) - !BitCheck(register_p, STATUS_BIT_CARRY));
			/* Check carry/unsigned overflow. */
			SetFlag(STATUS_BIT_CARRY, ((result16 & 0x100) == 0));
			/* Check signed overflow. */
			SetFlag(STATUS_BIT_OVERFLOW, ((register_a ^ result16) & (~Read((operand2 << 8) + operand1 + register_y) ^ result16) & 0x80));
			register_a = static_cast<uint8_t>(result16);
			UpdateZeroNegative(register_a);
			program_counter += 2;
			// TODO: Add 1 to cycle count if crossing page boundary.
			break;
		case 0xFA:
			illegal_opcode_triggered = true;
			/* NOP */
			break;
		case 0xFB:
			illegal_opcode_triggered = true;
			/* a,Y */
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			/* INC */
			result = Read((operand2 << 8) + operand1 + register_y);
			result++;
			Write((operand2 << 8) + operand1 + register_y, result);
			UpdateZeroNegative(result);
			/* SBC */
			result16 = (register_a - Read((operand2 << 8) + operand1 + register_y) - !BitCheck(register_p, STATUS_BIT_CARRY));
			/* Check carry/unsigned overflow. */
			SetFlag(STATUS_BIT_CARRY, ((result16 & 0x100) == 0));
			/* Check signed overflow. */
			SetFlag(STATUS_BIT_OVERFLOW, ((register_a ^ result16) & (~(Read((operand2 << 8) + operand1 + register_y)) ^ result16) & 0x80));
			register_a = static_cast<uint8_t>(result16);
			UpdateZeroNegative(register_a);
			program_counter += 2;
			break;
		case 0xFC:
			illegal_opcode_triggered = true;
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			Read((operand2 << 8) + operand1 + register_x);
			/* NOP */
			// TODO: Add 1 to cycle count if crossing page boundary.
			break;
		case 0xFD:
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			result = (register_a - Read((operand2 << 8) + operand1 + register_x) - !BitCheck(register_p, STATUS_BIT_CARRY));
			/* Check carry/unsigned overflow. */
			SetFlag(STATUS_BIT_CARRY, ((result & 0x100) == 0));
			/* Check signed overflow. */
			SetFlag(STATUS_BIT_OVERFLOW, ((register_a ^ result) & (~Read((operand2 << 8) + operand1 + register_x) ^ result) & 0x80));
			register_a = static_cast<uint8_t>(result);
			UpdateZeroNegative(register_a);
			program_counter += 2;
			// TODO: Add 1 to cycle count if crossing page boundary.
			break;
		case 0xFE:
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			result = Read((operand2 << 8) + operand1 + register_x);
			result++;
			Write(((operand2 << 8) + operand1 + register_x), result);
			UpdateZeroNegative(result);
			program_counter += 2;
			break;
		case 0xFF:
			illegal_opcode_triggered = true;
			/* a,X */
			operand1 = Read(program_counter + 1);
			operand2 = Read(program_counter + 2);
			/* INC */
			result = Read((operand2 << 8) + operand1 + register_x);
			result++;
			Write((operand2 << 8) + operand1 + register_x, result);
			UpdateZeroNegative(result);
			/* SBC */
			result16 = (register_a - result - !BitCheck(register_p, STATUS_BIT_CARRY));
			/* Check carry/unsigned overflow. */
			SetFlag(STATUS_BIT_CARRY, ((result16 & 0x100) == 0));
			/* Check signed overflow. */
			SetFlag(STATUS_BIT_OVERFLOW, ((register_a ^ result16) & (~(result) ^ result16) & 0x80));
			register_a = static_cast<uint8_t>(result16);
			UpdateZeroNegative(register_a);
			program_counter += 2;
			break;
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