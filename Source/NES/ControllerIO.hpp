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

#ifndef __CONTROLLER_IO_HPP__
#define __CONTROLLER_IO_HPP__

#include <cstdint>

class NESSystem;

#define NES_CONTROLLER_A      0
#define NES_CONTROLLER_B      1
#define NES_CONTROLLER_SELECT 2
#define NES_CONTROLLER_START  3
#define NES_CONTROLLER_UP     4
#define NES_CONTROLLER_DOWN   5
#define NES_CONTROLLER_LEFT   6
#define NES_CONTROLLER_RIGHT  7

typedef enum {
	NONE,
	NES_CONTROLLER,
	NES_ZAPPER,
	MIRACLE_PIANO,
	FAMILY_BASIC_KEYBOARD
} controller_type;

typedef struct {
	uint8_t controller_state_port1_D0;
	uint8_t controller_state_port1_D1;	
	uint8_t controller_state_port1_D2;
	uint8_t controller_state_port1_D3;
	uint8_t controller_state_port1_D4;
	
	uint8_t controller_state_port2_D0;
	uint8_t controller_state_port2_D1;
	uint8_t controller_state_port2_D2;
	uint8_t controller_state_port2_D3;
	uint8_t controller_state_port2_D4;
} controller_port_state;

/**
 * Controls both controller ports, and expansion port IO.
 */
class ControllerIO {

	public:
		ControllerIO(NESSystem*);
		~ControllerIO();

		void Initialize();
		void Shutdown();
		void Reset();

		uint8_t ReadIO(uint16_t address);
		void WriteIO(uint16_t address, uint8_t value);

		controller_port_state* GetControllerState() { return &controller_states; };

		bool IsControllerStrobeLatchHigh() { return controller_port_latch; };
		bool IsExpansion1StrobeLatchHigh() { return expansion_port_latch1; }
		bool IsExpansion2StrobeLatchHigh() { return expansion_port_latch2; }

	private:
		NESSystem* nes_system;

		controller_port_state controller_states;

		controller_type controller_port1;
		controller_type controller_port2;

		bool controller_port_latch;
		bool expansion_port_latch1;
		bool expansion_port_latch2;
};

#endif /* __CONTROLLER_IO_HPP__ */