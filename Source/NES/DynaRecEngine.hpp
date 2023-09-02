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

 #ifndef __DYNA_REC_ENGINE_HPP__
 #define __DYNA_REC_ENGINE_HPP__

#include <cstddef>
#include <cstdint>

using std::uint8_t;
using std::uint16_t;
using std::size_t;

class NESSystem;

class DynaRecEngine {

    public:

        struct Emulated {
            uint8_t register_a;
            uint8_t register_x;
            uint8_t register_y;
            uint8_t register_sp;
            uint8_t register_st;
            uint16_t register_ip;
        };

        struct Native {
            uint8_t* code_pointer;
            size_t code_size;
        };

        DynaRecEngine(NESSystem* nes_system);
        ~DynaRecEngine();
 
        void Initialize();
		void Shutdown();
		void Reset(bool hard);

        void Step();
        void Run();

    private:
        NESSystem* nes_system;

		uint8_t cpu_memory[0x800] { 0 };

        Emulated emulated_cpu;
};

 #endif /* __DYNA_REC_ENGINE_HPP__ */