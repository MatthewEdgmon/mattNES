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

#include "APU.hpp"

#include <iostream>

#include "../HexOutput.hpp"
#include "NESSystem.hpp"
#include "CPU.hpp"

APU::APU(NESSystem* nes_system) : nes_system(nes_system) {

}

APU::~APU() {

}

void APU::Initialize() {

}

void APU::Shutdown() {

}

void APU::Reset(bool hard) {

}

void APU::Step() {

}

uint8_t APU::ReadAPU(uint16_t address) {
	return 0xFF;
}

void APU::WriteAPU(uint16_t address, uint8_t value) {

}

uint8_t APU::ReadCPU(uint16_t address) {

	uint8_t value = nes_system->GetFloatingBus();

	if(address == 0x4000) { value = sq1_volume; }
	if(address == 0x4001) { value = sq1_sweep; }
	if(address == 0x4002) { value = sq1_period_low; }
	if(address == 0x4003) { value = sq1_period_high; }

	if(address == 0x4004) { value = sq2_volume; }
	if(address == 0x4005) { value = sq2_sweep; }
	if(address == 0x4006) { value = sq2_period_low; }
	if(address == 0x4007) { value = sq2_period_high; }

	if(address == 0x4008) { value = tri_counter; }
	if(address == 0x4009) { value = unused1; }
	if(address == 0x400A) { value = tri_period_low; }
	if(address == 0x400B) { value = tri_period_high; }

	if(address == 0x400C) { value = noise_volume; }
	if(address == 0x400D) { value = unused2; }
	if(address == 0x400E) { value = noise_period_low; }
	if(address == 0x400F) { value = noise_period_high; }

	if(address == 0x4010) { value = dmc_frequency; }
	if(address == 0x4011) { value = dmc_raw; }
	if(address == 0x4012) { value = dmc_start; }
	if(address == 0x4013) { value = dmc_length; }

	if(address == 0x4015) { value = status_register; }

	if(address == 0x4017) { value = frame_counter; }
	
	if(nes_system->GetCPU()->IsInTestMode()) {
		if(address == 0x4018) {
			value = 0xFF; // Current instant DAC value of pulse B (from 0 to current volume, bits 4 - 7) or pulse A (from 0 to current volume, bits 0 to 3).
		}
		
		if(address == 0x4019) {
			value = 0xFF; // Current instant DAC value of noise (from 0 to current volume, bits 4 - 7) or triangle (from 0 to 15, bits 0 to 3).
		}
		
		if(address == 0x401A) {
			value = 0xFF; // Current instant DAC value of DPCM channel (excluding bit 7).
		}
	} else {
		return nes_system->GetFloatingBus();
	}

	/* Disabled APU and IO functionality. */
	if(address >= 0x401B && address <= 0x401F) {
		std::cout << "Reading from completely disabled APU/IO area." << std::endl;
		return value;
	}

	nes_system->SetFloatingBus(value);
	return value;
}

void APU::WriteCPU(uint16_t address, uint8_t value) {

	nes_system->SetFloatingBus(value);

	if(address == 0x4000) { sq1_volume = value; return; }
	if(address == 0x4001) { sq1_sweep = value; return; }
	if(address == 0x4002) { sq1_period_low = value; return; }
	if(address == 0x4003) { sq1_period_high = value; return; }

	if(address == 0x4004) { sq2_volume = value; return; }
	if(address == 0x4005) { sq2_sweep = value; return; }
	if(address == 0x4006) { sq2_period_low = value; return; }
	if(address == 0x4007) { sq2_period_high = value; return; }

	if(address == 0x4008) { tri_counter = value; return; }
	if(address == 0x4009) { unused1 = value; return; }
	if(address == 0x400A) { tri_period_low = value; return; }
	if(address == 0x400B) { tri_period_high = value; return; }

	if(address == 0x400C) { noise_volume = value; return; }
	if(address == 0x400D) { unused2 = value; return; }
	if(address == 0x400E) { noise_period_low = value; return; }
	if(address == 0x400F) { noise_period_high = value; return; }

	if(address == 0x4010) { dmc_frequency = value; return; }
	if(address == 0x4011) { dmc_raw = value; return; }
	if(address == 0x4012) { dmc_start = value; return; }
	if(address == 0x4013) { dmc_length = value; return; }

	if(address == 0x4015) { status_register = value; return; }

	if(address == 0x4017) { frame_counter = value; return; }

	if(nes_system->GetCPU()->IsInTestMode()) {
		if(address == 0x4018) {
			// Do nothing.
			return;
		}
		
		if(address == 0x4019) {
			// Do nothing.
			return;
		}
		
		if(address == 0x401A) {
			// Bit 7 is "Lock Channel Output". Bottom 5 bits set current instant phase of triangle.
			return;
		}
	}

	/* Disabled APU and IO functionality. */
	if(address >= 0x401B && address <= 0x401F) {
		std::cout << "Writing to completely disabled APU/IO area." << std::endl;
		return;
	}

	return;
}