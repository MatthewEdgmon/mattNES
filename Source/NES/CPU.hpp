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

#define STATUS_BIT_CARRY             0
#define STATUS_BIT_ZERO              1
#define STATUS_BIT_INTERRUPT_DISABLE 2
#define STATUS_BIT_DECIMAL           3
#define STATUS_BIT_S1                4
#define STATUS_BIT_S2                5
#define STATUS_BIT_OVERFLOW          6
#define STATUS_BIT_NEGATIVE          7

class NESSystem;

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

		void Push(uint8_t value);
		uint8_t Pop();

		void PerformOAMDMA(uint8_t value);

		void GenerateNMI();
		void GenerateIRQ();

		/* Read from CPU memory without causing any emulation side effects. */
		uint8_t ReadDebug(uint16_t address);
		/* Read from CPU stack without causing any emulation side effects. */
		uint8_t PeekStack(uint8_t stack_location) { return cpu_memory[(0x100 + stack_location)]; };

		uint16_t GetProgramCounter() { return program_counter; };
		uint8_t GetRegisterP() { return register_p; };
		uint8_t GetRegisterA() { return register_a; };
		uint8_t GetRegisterX() { return register_x; };
		uint8_t GetRegisterY() { return register_y; };
		uint8_t GetRegisterS() { return register_s; };

		bool IsInTestMode() { return test_mode; };

		void ToggleDisassembly() { show_disassembly = !show_disassembly; };

		uint64_t CycleCount() { return cycles; };

	private:
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
		bool increment_pc;

		bool test_mode;

		bool show_disassembly;

		uint64_t cycles;

	private:
		typedef enum AddressingMode {
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

		typedef enum Opcode {
			ADC, AND, ASL, BCC, BCS, BEQ, BIT, BMI, BNE, BPL, BRK, BVC, BVS, CLC,
			CLD, CLI, CLV, CMP, CPX, CPY, DEC, DEX, DEY, EOR, INC, INX, INY, JMP,
			JSR, LDA, LDX, LDY, LSR, NOP, ORA, PHA, PHP, PLA, PLP, ROL, ROR, RTI,
			RTS, SBC, SEC, SED, SEI, STA, STX, STY, TAX, TAY, TSX, TXA, TXS, TYA
		} opcode_t;
};

#endif /* __CPU_HPP__ */