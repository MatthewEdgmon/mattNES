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

 #ifndef __CPU_HPP__
 #define __CPU_HPP__

#include <cstddef>
#include <cstdint>
#include <map>
#include <string>
#include <vector>

#define STATUS_BIT_CARRY             0
#define STATUS_BIT_ZERO              1
#define STATUS_BIT_INTERRUPT_DISABLE 2
#define STATUS_BIT_DECIMAL           3
#define STATUS_BIT_S1                4
#define STATUS_BIT_S2                5
#define STATUS_BIT_OVERFLOW          6
#define STATUS_BIT_NEGATIVE          7

using std::uint8_t;
using std::uint16_t;
using std::uint64_t;
using std::size_t;

class NESSystem;

class CPU {

    public:
        // TODO: Potentially move everything "state" into one struct here.

        typedef enum InterruptType {
	        INTERRUPT_NMI,
	        INTERRUPT_IRQ,
	        INTERRUPT_BRK
        } interrupt_type_t;

        typedef struct NativeCode {
            uint8_t* code_pointer;
            int code_size;
        } native_code_t;

        CPU(NESSystem* nes_system, std::vector<uint8_t> state);
        CPU(NESSystem* nes_system);
        ~CPU();
 
        void Initialize();
		void Shutdown();
		void Reset(bool hard);

        void ValidateCache(uint16_t address, uint8_t value);
        void Compile(uint16_t address);
        void Step();
        void Run();
        void Interrupt(interrupt_type_t type);

        std::vector<uint8_t> SaveState();
        void LoadState(std::vector<uint8_t> state);

        std::string DebugInfo();

        uint16_t GetProgramCounter() { return program_counter; };
		uint8_t GetRegisterP() { return register_p; };
		uint8_t GetRegisterA() { return register_a; };
		uint8_t GetRegisterX() { return register_x; };
		uint8_t GetRegisterY() { return register_y; };
		uint8_t GetRegisterS() { return register_s; };

        bool IsInTestMode() { return test_mode; };

        void SetProgramCounter(uint16_t value) { program_counter = value; };

        void ToggleOutput() { output_decode = !output_decode; };

    private:
        NESSystem* nes_system;

        /* Cache of decoded instruction blocks. */
        std::map<uint16_t, native_code_t> code_cache;

		/* Vectors */
		uint16_t vector_nmi { 0 };
		uint16_t vector_irq { 0 };
		uint16_t vector_rst { 0 };

		/* CPU registers */
		uint16_t program_counter { 0 };
		uint8_t register_a { 0 };
		uint8_t register_x { 0 };
		uint8_t register_y { 0 };
		uint8_t register_s { 0 };
        uint8_t register_p { 0 };

        /* CPU Test Mode */
        bool test_mode { false };

        /* Flag to enable output of decoded instructions. */
        bool output_decode { false };
};

 #endif /* __CPU_HPP__ */