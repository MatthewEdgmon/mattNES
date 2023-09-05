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

#include "../BitOps.hpp"
#include "../HexOutput.hpp"
#include "NESSystem.hpp"
#include "ControllerIO.hpp"

ControllerIO::ControllerIO(NESSystem* nes_system) : nes_system(nes_system) {

}

ControllerIO::~ControllerIO() {

}

void ControllerIO::Initialize() {
	controller_port1 = NES_CONTROLLER;
	controller_port2 = NONE;
	controller_states = { 0 };
}

void ControllerIO::Shutdown() {

}

void ControllerIO::Reset() {
	controller_states = { 0 };
}

uint8_t ControllerIO::ReadIO(uint16_t address) {

	uint8_t value = nes_system->GetFloatingBus();

	// TODO: Turn this into a switch statement for other controller devices.

	/* Controller Port 1 */
	if(address == 0x4016) {
		/* Send CLK pulse to shift bits. */


		/* Capture D0 */
		/* Capture D1 */
		/* Capture D2 */
		/* Capture D3 */
		/* Capture D4 */
	}
	
	/* Controller Port 2 */
	if(address == 0x4017) {
		/* Send CLK pulse to shift bits. */


		/* Capture D0 */
		/* Capture D1 */
		/* Capture D2 */
		/* Capture D3 */
		/* Capture D4 */
	}

	nes_system->SetFloatingBus(value);
	return value;
}

void ControllerIO::WriteIO(uint16_t address, uint8_t value) {

	nes_system->SetFloatingBus(value);

	if(address == 0x4016) {
		if(value & 0x01) { controller_port_latch = 1; } else { controller_port_latch = 0; }
		if(value & 0x02) { expansion_port_latch1 = 1; } else { expansion_port_latch1 = 0; }
		if(value & 0x03) { expansion_port_latch2 = 1; } else { expansion_port_latch2 = 0; }
	}

	return;
}