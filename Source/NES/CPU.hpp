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

#ifndef __CPU_HPP__
#define __CPU_HPP__

#include <bitset>
#include <cstdint>
#include <vector>

#include "../BitOps.hpp"

#define STATUS_BIT_CARRY             0
#define STATUS_BIT_ZERO              1
#define STATUS_BIT_INTERRUPT_DISABLE 2
#define STATUS_BIT_DECIMAL           3
#define STATUS_BIT_S1                4
#define STATUS_BIT_S2                5
#define STATUS_BIT_OVERFLOW          6
#define STATUS_BIT_NEGATIVE          7

class NESSystem;

typedef enum addressing_mode {
	IMP,
	IMM,
	ZPG,
	ZPX,
	ZPY,
	REL,
	ABS,
	ABX,
	ABY,
	IND,
	IIN,
	INI
} addressing_mode_t;

typedef enum interrupt_type {
	INTERRUPT_NMI,
	INTERRUPT_IRQ,
	INTERRUPT_BRK
} interrupt_type_t;

typedef enum opcode {
	ADC, AND, ASL, BCC, BCS, BEQ, BIT, BMI, BNE, BPL, BRK, BVC, BVS, CLC,
	CLD, CLI, CLV, CMP, CPX, CPY, DEC, DEX, DEY, EOR, INC, INX, INY, JMP,
	JSR, LDA, LDX, LDY, LSR, NOP, ORA, PHA, PHP, PLA, PLP, ROL, ROR, RTI,
	RTS, SBC, SEC, SED, SEI, STA, STX, STY, TAX, TAY, TSX, TXA, TXS, TYA
} opcode_t;

class CPU {

	public:
		CPU(NESSystem* nes_system);
		~CPU();

		void Initialize();
		void Shutdown();
		void Reset(bool hard);

		/* Functions located in CPU_Instructions.cpp ----------------------------------------------------- */

		void Step();             /* Execute one instruction. */
		void IllegalOpcode(uint8_t opcode);

		/* Functions located in CPU_Disassemble.cpp ------------------------------------------------------ */

		void StepDisassembler(); /* Ouput debugging information for the current step. */

		/* ----------------------------------------------------------------------------------------------- */

		uint8_t Read(uint16_t address);
		void Write(uint16_t address, uint8_t value);

		/* Read from CPU memory without causing any emulation side effects. */
		uint8_t PeekMemory(uint16_t address);

		void Push(uint8_t value);
		uint8_t Pop();

		void Interrupt(interrupt_type_t interrupt_type);

		void PerformOAMDMA(uint8_t value);

		uint16_t GetProgramCounter() { return program_counter; };
		uint8_t GetRegisterP() { return register_p; };
		uint8_t GetRegisterA() { return register_a; };
		uint8_t GetRegisterX() { return register_x; };
		uint8_t GetRegisterY() { return register_y; };
		uint8_t GetRegisterS() { return register_s; };

		void SetProgramCounter(uint16_t value) { program_counter = value; };

		bool IsInTestMode() { return test_mode; };

		void ToggleDisassembly() { show_disassembly = !show_disassembly; };

		uint64_t CycleCount() { return cycles; };

	private:
		/* Updates CPU flags based on input value. */
		void UpdateZeroNegative(uint8_t value) {
			if(value == 0x00) { BitSet(register_p, STATUS_BIT_ZERO); }     else { BitClear(register_p, STATUS_BIT_ZERO); }
			if(value >= 0x80) { BitSet(register_p, STATUS_BIT_NEGATIVE); } else { BitClear(register_p, STATUS_BIT_NEGATIVE); }
		};

		/* For certain instructions, crossing a page boundary costs extra cycle(s). */
		void CheckPageCross(uint16_t address_a, uint16_t address_b, uint8_t add_cycles) {
			/* Determine page cross if high byte of two addresses differ. */
			if ((address_a & 0xFF00) != (address_b & 0xFF00)) {
				cycles += add_cycles;
			}
		}

		/* Sets a specified bit in the flag register to the value of condition. */
		void SetFlag(uint8_t flag_bit, bool condition) {
			if(condition) {
				BitSet(register_p, flag_bit);
			} else {
				BitClear(register_p, flag_bit);
			}
		}

		NESSystem* nes_system;

		uint8_t cpu_memory[0x800];

		/* Vectors */
		uint16_t vector_nmi;
		uint16_t vector_irq;
		uint16_t vector_rst;

		/* CPU registers */
		uint16_t program_counter;
		uint8_t register_p;
		uint8_t register_a;
		uint8_t register_x;
		uint8_t register_y;
		uint8_t register_s;

		/* instruction being executed at current step. */
		uint8_t instruction;

		/* Argument storage for instructions. */
		uint8_t operand1;
		uint8_t operand2;

		/* Argument storage for disassembly. */
		uint8_t dis_operand1;
		uint8_t dis_operand2;

		bool halted;
		bool illegal_opcode_triggered;
		bool halt_on_illegal_opcode;
		bool increment_pc;

		bool test_mode;

		bool show_disassembly;

		uint64_t cycles;

		/* How many clock cycles each instruction takes. */
		uint8_t cycle_sizes[0x100] = {
			7, 6, 0, 0, 0, 3, 5, 0, 3, 2, 2, 0, 0, 4, 6, 0,
            2, 5, 0, 0, 0, 4, 6, 0, 2, 4, 0, 0, 0, 4, 7, 0,
            6, 6, 0, 0, 3, 3, 5, 0, 4, 2, 2, 0, 4, 4, 6, 0,
            2, 5, 0, 0, 0, 4, 6, 0, 2, 4, 0, 0, 0, 4, 7, 0,
            6, 6, 0, 0, 0, 3, 5, 0, 3, 2, 2, 0, 3, 4, 6, 0,
            2, 5, 0, 0, 0, 4, 6, 0, 2, 4, 0, 0, 0, 4, 7, 0,
            6, 6, 0, 0, 0, 3, 5, 0, 4, 2, 2, 0, 5, 4, 6, 0,
            2, 5, 0, 0, 0, 4, 6, 0, 2, 4, 0, 0, 0, 4, 7, 0,
            0, 6, 0, 0, 3, 3, 3, 0, 2, 0, 2, 0, 4, 4, 4, 0,
            2, 6, 0, 0, 4, 4, 4, 0, 2, 5, 2, 0, 0, 5, 0, 0,
            2, 6, 2, 0, 3, 3, 3, 0, 2, 2, 2, 0, 4, 4, 4, 0,
            2, 5, 0, 0, 4, 4, 4, 0, 2, 4, 2, 0, 4, 4, 4, 0,
            2, 6, 0, 0, 3, 3, 5, 0, 2, 2, 2, 0, 4, 4, 6, 0,
            2, 5, 0, 0, 0, 4, 6, 0, 2, 4, 0, 0, 0, 4, 7, 0,
            2, 6, 0, 0, 3, 3, 5, 0, 2, 2, 2, 2, 4, 4, 6, 0,
            2, 5, 0, 0, 0, 4, 6, 0, 2, 4, 0, 0, 0, 4, 7, 0
		};

		/* Size of each instruction in bytes. */
		uint8_t instruction_sizes[0x100] = {
			1, 2, // TODO: Fill in the rest of this struct
			// TODO: Fix HACK
			// TODO: Finish changing flag setting to SetFlag
			// TODO: Modify PHP 0x08 to push correct value on bits 4 & 5
			// TODO: Seperate Interrupt() into two, one for requesting the interrupt, one for actually handling it, and a bool owned by the class for checking whether an interrupt is pending
		};
};

#endif /* __CPU_HPP__ */