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

typedef enum interrupt_type {
	INTERRUPT_NMI,
	INTERRUPT_IRQ,
	INTERRUPT_BRK
} interrupt_type_t;

typedef enum addressing_mode {
	IMP,
	ACU,
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

typedef enum opcode {
	/* Offical */
	ADC, AND, ASL, BCC, BCS, BEQ, BIT, BMI, BNE, BPL, BRK, BVC, BVS, CLC,
	CLD, CLI, CLV, CMP, CPX, CPY, DEC, DEX, DEY, EOR, INC, INX, INY, JMP,
	JSR, LDA, LDX, LDY, LSR, NOP, ORA, PHA, PHP, PLA, PLP, ROL, ROR, RTI,
	RTS, SBC, SEC, SED, SEI, STA, STX, STY, TAX, TAY, TSX, TXA, TXS, TYA,

	/* Illegal, using naming convention from NESdev wiki. */
	AHX, ALR, ANC, ARR, AXS, DCP, ISC, LAS, LAX, RLA, RRA, SAX, SLO, SRE,
	TAS, SHX, SHY, STP, XAA
} opcode_t;

class CPU {

	public:
		CPU(NESSystem* nes_system);
		~CPU();

		void Initialize();
		void Shutdown();
		void Reset(bool hard);

		/* Functions located in CPU_Instructions.cpp ----------------------------------------------------- */

		/* Execute one instruction. */
		void Step();

		/* Functions located in CPU_Disassemble.cpp ------------------------------------------------------ */

		/* Output disassembly information to a string. */
		std::string StepDisassembler();

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

		/* Instruction being executed at current step. */
		uint8_t instruction;

		/* Argument storage for instructions. */
		uint8_t operand1;
		uint8_t operand2;

		/* Instruction storage for disassembly. */
		uint8_t dis_instruction;

		/* Argument storage for disassembly. */
		uint8_t dis_operand1;
		uint8_t dis_operand2;

		bool halted;
		bool illegal_opcode_triggered;
		bool halt_on_illegal_opcode;
		bool increment_pc;

		bool test_mode;

		uint64_t cycles;

		/* Associates instruction byte with instruction names. */
		std::string instruction_names[0x100] = {
			"BRK", "ORA", "STP", "SLO", "NOP", "ORA", "ASL", "SLO", "PHP", "ORA", "ASL", "ANC", "NOP", "ORA", "ASL", "SLO",
			"BPL", "ORA", "STP", "SLO", "NOP", "ORA", "ASL", "SLO", "CLC", "ORA", "NOP", "SLO", "NOP", "ORA", "ASL", "SLO",
			"JSR", "AND", "STP", "RLA", "BIT", "AND", "ROL", "RLA", "PLP", "AND", "ROL", "ANC", "BIT", "AND", "ROL", "RLA",
			"BMI", "AND", "STP", "RLA", "NOP", "AND", "ROL", "RLA", "SEC", "AND", "NOP", "RLA", "NOP", "AND", "ROL", "RLA",
			"RTI", "EOR", "STP", "SRE", "NOP", "EOR", "LSR", "SRE", "PHA", "EOR", "LSR", "ALR", "JMP", "EOR", "LSR", "SRE",
			"BVC", "EOR", "STP", "SRE", "NOP", "EOR", "LSR", "SRE", "CLI", "EOR", "NOP", "SRE", "NOP", "EOR", "LSR", "SRE",
			"RTS", "ADC", "STP", "RRA", "NOP", "ADC", "ROR", "RRA", "PLA", "ADC", "ROR", "ARR", "JMP", "ADC", "ROR", "RRA",
			"BVS", "ADC", "STP", "RRA", "NOP", "ADC", "ROR", "RRA", "SEI", "ADC", "NOP", "RRA", "NOP", "ADC", "ROR", "RRA",
			"NOP", "STA", "NOP", "SAX", "STY", "STA", "STX", "SAX", "DEY", "NOP", "TXA", "XAA", "STY", "STA", "STX", "SAX",
			"BCC", "STA", "STP", "AHX", "STY", "STA", "STX", "SAX", "TYA", "STA", "TXS", "TAS", "SHY", "STA", "SHX", "AHX",
			"LDY", "LDA", "LDX", "LAX", "LDY", "LDA", "LDX", "LAX", "TAY", "LDA", "TAX", "LAX", "LDY", "LDA", "LDX", "LAX",
			"BCS", "LDA", "STP", "LAX", "LDY", "LDA", "LDX", "LAX", "CLV", "LDA", "TSX", "LAS", "LDY", "LDA", "LDX", "LAX",
			"CPY", "CMP", "NOP", "DCP", "CPY", "CMP", "DEC", "DCP", "INY", "CMP", "DEX", "AXS", "CPY", "CMP", "DEC", "DCP",
			"BNE", "CMP", "STP", "DCP", "NOP", "CMP", "DEC", "DCP", "CLD", "CMP", "NOP", "DCP", "NOP", "CMP", "DEC", "DCP",
			"CPX", "SBC", "NOP", "ISB", "CPX", "SBC", "INC", "ISB", "INX", "SBC", "NOP", "SBC", "CPX", "SBC", "INC", "ISB",
			"BEQ", "SBC", "STP", "ISB", "NOP", "SBC", "INC", "ISB", "SED", "SBC", "NOP", "ISB", "NOP", "SBC", "INC", "ISB"
		};

		/* Associates instruction byte with instruction opcode. */
		opcode_t instruction_opcode[0x100]{
			BRK, ORA, STP, SLO, NOP, ORA, ASL, SLO, PHP, ORA, ASL, ANC, NOP, ORA, ASL, SLO,
			BPL, ORA, STP, SLO, NOP, ORA, ASL, SLO, CLC, ORA, NOP, SLO, NOP, ORA, ASL, SLO,
			JSR, AND, STP, RLA, BIT, AND, ROL, RLA, PLP, AND, ROL, ANC, BIT, AND, ROL, RLA,
			BMI, AND, STP, RLA, NOP, AND, ROL, RLA, SEC, AND, NOP, RLA, NOP, AND, ROL, RLA,
			RTI, EOR, STP, SRE, NOP, EOR, LSR, SRE, PHA, EOR, LSR, ALR, JMP, EOR, LSR, SRE,
			BVC, EOR, STP, SRE, NOP, EOR, LSR, SRE, CLI, EOR, NOP, SRE, NOP, EOR, LSR, SRE,
			RTS, ADC, STP, RRA, NOP, ADC, ROR, RRA, PLA, ADC, ROR, ARR, JMP, ADC, ROR, RRA,
			BVS, ADC, STP, RRA, NOP, ADC, ROR, RRA, SEI, ADC, NOP, RRA, NOP, ADC, ROR, RRA,
			NOP, STA, NOP, SAX, STY, STA, STX, SAX, DEY, NOP, TXA, XAA, STY, STA, STX, SAX,
			BCC, STA, STP, AHX, STY, STA, STX, SAX, TYA, STA, TXS, TAS, SHY, STA, SHX, AHX,
			LDY, LDA, LDX, LAX, LDY, LDA, LDX, LAX, TAY, LDA, TAX, LAX, LDY, LDA, LDX, LAX,
			BCS, LDA, STP, LAX, LDY, LDA, LDX, LAX, CLV, LDA, TSX, LAS, LDY, LDA, LDX, LAX,
			CPY, CMP, NOP, DCP, CPY, CMP, DEC, DCP, INY, CMP, DEX, AXS, CPY, CMP, DEC, DCP,
			BNE, CMP, STP, DCP, NOP, CMP, DEC, DCP, CLD, CMP, NOP, DCP, NOP, CMP, DEC, DCP,
			CPX, SBC, NOP, ISC, CPX, SBC, INC, ISC, INX, SBC, NOP, SBC, CPX, SBC, INC, ISC,
			BEQ, SBC, STP, ISC, NOP, SBC, INC, ISC, SED, SBC, NOP, ISC, NOP, SBC, INC, ISC
		};

		/* Defines addressing mode for each opcode. */
		addressing_mode_t instruction_mode[0x100] = {
			IMP, IIN, IMP, IIN, ZPG, ZPG, ZPG, ZPG, IMP, IMM, ACU, IMM, ABS, ABS, ABS, ABS,
			REL, INI, IMP, INI, ZPX, ZPX, ZPX, ZPX, IMP, ABY, IMP, ABY, ABX, ABX, ABX, ABX,
			ABS, IIN, IMP, IIN, ZPG, ZPG, ZPG, ZPG, IMP, IMM, ACU, IMM, ABS, ABS, ABS, ABS,
			REL, INI, IMP, INI, ZPX, ZPX, ZPX, ZPX, IMP, ABY, IMP, ABY, ABX, ABX, ABX, ABX,
			IMP, IIN, IMP, IIN, ZPG, ZPG, ZPG, ZPG, IMP, IMM, ACU, IMM, ABS, ABS, ABS, ABS,
			REL, INI, IMP, INI, ZPX, ZPX, ZPX, ZPX, IMP, ABY, IMP, ABY, ABX, ABX, ABX, ABX,
			IMP, IIN, IMP, IIN, ZPG, ZPG, ZPG, ZPG, IMP, IMM, ACU, IMM, IND, ABS, ABS, ABS,
			REL, INI, IMP, INI, ZPX, ZPX, ZPX, ZPX, IMP, ABY, IMP, ABY, ABX, ABX, ABX, ABX,
			IMM, IIN, IMM, IIN, ZPG, ZPG, ZPG, ZPG, IMP, IMM, IMP, IMM, ABS, ABS, ABS, ABS,
			REL, INI, IMP, INI, ZPX, ZPX, ZPY, ZPY, IMP, ABY, IMP, ABY, ABX, ABX, ABY, ABY,
			IMM, IIN, IMM, IIN, ZPG, ZPG, ZPG, ZPG, IMP, IMM, IMP, IMM, ABS, ABS, ABS, ABS,
			REL, INI, IMP, INI, ZPX, ZPX, ZPY, ZPY, IMP, ABY, IMP, ABY, ABX, ABX, ABY, ABY,
			IMM, IIN, IMM, IIN, ZPG, ZPG, ZPG, ZPG, IMP, IMM, IMP, IMM, ABS, ABS, ABS, ABS,
			REL, INI, IMP, INI, ZPX, ZPX, ZPX, ZPX, IMP, ABY, IMP, ABY, ABX, ABX, ABX, ABX,
			IMM, IIN, IMM, IIN, ZPG, ZPG, ZPG, ZPG, IMP, IMM, IMP, IMM, ABS, ABS, ABS, ABS,
			REL, INI, IMP, INI, ZPX, ZPX, ZPX, ZPX, IMP, ABY, IMP, ABY, ABX, ABX, ABX, ABX
		};

		/* How many clock cycles each instruction takes. */
		// TODO: Cycles of 0 means STP, and halt processor.
		uint8_t cycle_sizes[0x100] = {
			7, 6, 0, 8, 0, 3, 5, 5, 3, 2, 2, 2, 0, 4, 6, 6,
            2, 5, 0, 8, 0, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7,
            6, 6, 0, 8, 3, 3, 5, 5, 4, 2, 2, 2, 4, 4, 6, 6,
            2, 5, 0, 8, 0, 4, 6, 6, 2, 4, 0, 7, 0, 4, 7, 7,
            6, 6, 0, 8, 0, 3, 5, 5, 3, 2, 2, 2, 3, 4, 6, 6,
            2, 5, 0, 0, 0, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7,
            6, 6, 0, 8, 0, 3, 5, 5, 4, 2, 2, 2, 5, 4, 6, 6,
            2, 5, 0, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7,
            2, 6, 2, 6, 3, 3, 3, 3, 2, 2, 2, 0, 4, 4, 4, 4,
            2, 6, 0, 0, 4, 4, 4, 4, 2, 5, 2, 0, 0, 5, 0, 0,
            2, 6, 2, 6, 3, 3, 3, 3, 2, 2, 2, 2, 4, 4, 4, 4,
            2, 5, 0, 5, 4, 4, 4, 4, 2, 4, 2, 0, 4, 4, 4, 4,
            2, 6, 2, 8, 3, 3, 5, 5, 2, 2, 2, 2, 4, 4, 6, 6,
            2, 5, 0, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7,
            2, 6, 2, 8, 3, 3, 5, 0, 2, 2, 2, 2, 4, 4, 6, 6,
            2, 5, 0, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7
		};

		/* Size of each instruction in bytes. */
		uint8_t instruction_sizes[0x100] = {
			1, 2, 1, 2, 2, 2, 2, 2, 1, 2, 1, 2, 3, 3, 3, 3,
			2, 2, 1, 2, 2, 2, 2, 2, 1, 3, 1, 3, 3, 3, 3, 3,
			3, 2, 1, 2, 2, 2, 2, 2, 1, 2, 1, 2, 3, 3, 3, 3,
			2, 2, 1, 2, 2, 2, 2, 2, 1, 3, 1, 3, 3, 3, 3, 3,
			1, 2, 1, 2, 2, 2, 2, 2, 1, 2, 1, 2, 3, 3, 3, 3,
			2, 2, 1, 2, 2, 2, 2, 2, 1, 3, 1, 3, 3, 3, 3, 3,
			1, 2, 1, 2, 2, 2, 2, 2, 1, 2, 1, 2, 3, 3, 3, 3,
			2, 2, 1, 2, 2, 2, 2, 2, 1, 3, 1, 3, 3, 3, 3, 3,
			2, 2, 2, 2, 2, 2, 2, 2, 1, 2, 1, 2, 3, 3, 3, 3,
			2, 2, 1, 2, 2, 2, 2, 2, 1, 3, 1, 3, 3, 3, 3, 3,
			2, 2, 2, 2, 2, 2, 2, 2, 1, 2, 1, 2, 3, 3, 3, 3,
			2, 2, 1, 2, 2, 2, 2, 2, 1, 3, 1, 3, 3, 3, 3, 3,
			2, 2, 2, 2, 2, 2, 2, 2, 1, 2, 1, 2, 3, 3, 3, 3,
			2, 2, 1, 2, 2, 2, 2, 2, 1, 3, 1, 3, 3, 3, 3, 3,
			2, 2, 2, 2, 2, 2, 2, 2, 1, 2, 1, 2, 3, 3, 3, 3,
			2, 2, 1, 2, 2, 2, 2, 2, 1, 3, 1, 3, 3, 3, 3, 3
		};

		/* Is the instruction "illegal/unoffical/undocumented". */
		bool instruction_is_illegal[0x100]{
			0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 0, 1, 1, 0, 0, 1,
			0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 1, 1, 1, 0, 0, 1,
			0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1,
			0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 1, 1, 1, 0, 0, 1,
			0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1,
			0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 1, 1, 1, 0, 0, 1,
			0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1,
			0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 1, 1, 1, 0, 0, 1,
			1, 0, 1, 1, 0, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 1,
			0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, 1, 1, 0, 1, 1,
			0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1,
			0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1,
			0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1,
			0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 1, 1, 1, 0, 0, 1,
			0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1,
			0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 1, 1, 1, 0, 0, 1
		};
};

#endif /* __CPU_HPP__ */