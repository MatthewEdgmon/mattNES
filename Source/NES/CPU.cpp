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

#include <iostream>
#include <string>
#include <sstream>
#include <vector>

#include "../BitOps.hpp"
#include "../HexOutput.hpp"
#include "CPU.hpp"
#include "NESSystem.hpp"

CPU::CPU(NESSystem* nes_system) : nes_system(nes_system) {
    
}

CPU::~CPU() {

}

void CPU::Initialize() {

}

void CPU::Shutdown() {

}

void CPU::Reset(bool hard) {

}

void CPU::ValidateCache(uint16_t address, uint8_t value) {

}

void CPU::Compile(uint16_t address) {

}

void CPU::Step() {
    std::cout << HEX2(nes_system->Read(0x0000));
}

void CPU::Run() {
    
}

void CPU::Interrupt(interrupt_type_t type) {
    
}

std::vector<uint8_t> CPU::SaveState() {
    std::vector<uint8_t> state;

    return state;
}

void CPU::LoadState(std::vector<uint8_t> state) {

}

std::string CPU::DebugInfo() {

    std::ostringstream debug_output;

    debug_output << "[CPU]\n";
    debug_output << "program_counter" << HEX4(program_counter) << '\n';
    debug_output << "register_a = " << HEX2(register_a) << '\n';
    debug_output << "register_x = " << HEX2(register_x) << '\n';
    debug_output << "register_y = " << HEX2(register_y) << '\n';
    debug_output << "register_s = " << HEX2(register_s) << '\n';
    debug_output << "register_p = " << HEX2(register_p) << '\n';
	if(BitCheck(register_p, STATUS_BIT_NEGATIVE))          { debug_output << "NEGATIVE"; }   else { debug_output << "        "; }
	if(BitCheck(register_p, STATUS_BIT_OVERFLOW))          { debug_output << " OVERFLOW"; }  else { debug_output << "         "; }
	if(BitCheck(register_p, STATUS_BIT_S2))                { debug_output << " UNUSED2"; }   else { debug_output << "        "; }
	if(BitCheck(register_p, STATUS_BIT_S1))                { debug_output << " UNUSED1"; }   else { debug_output << "        "; }
	if(BitCheck(register_p, STATUS_BIT_DECIMAL))           { debug_output << " DECIMAL"; }   else { debug_output << "        "; }
	if(BitCheck(register_p, STATUS_BIT_INTERRUPT_DISABLE)) { debug_output << " INTERRUPT"; } else { debug_output << "          "; }
	if(BitCheck(register_p, STATUS_BIT_ZERO))              { debug_output << " ZERO"; }      else { debug_output << "     "; }
	if(BitCheck(register_p, STATUS_BIT_CARRY))             { debug_output << " CARRY"; }     else { debug_output << "      "; }
	debug_output << "\n";

    return debug_output.str();
}
