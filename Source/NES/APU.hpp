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

#ifndef __APU_HPP__
#define __APU_HPP__

#include <cstdint>
#include <vector>

class NESSystem;

class APU {

	public:
		APU(NESSystem* nes_system);
		~APU();

		void Initialize();
		void Shutdown();
		void Reset(bool hard);
		void Step();

		uint8_t ReadAPU(uint16_t address);
		void WriteAPU(uint16_t address, uint8_t value);

		uint8_t ReadCPU(uint16_t address);
		void WriteCPU(uint16_t address, uint8_t value);

	private:
		NESSystem* nes_system;

		uint8_t sq1_volume;        /* Duty and volume for square wave 1. */
		uint8_t sq1_sweep;         /* Sweep control register for square wave 1. */
		uint8_t sq1_period_low;    /* Low byte of period for square wave 1. */
		uint8_t sq1_period_high;   /* High byte of period and length counter value for square wave 1. */

		uint8_t sq2_volume;        /* Duty and volume for square wave 2. */
		uint8_t sq2_sweep;         /* Sweep control register for square wave 2. */
		uint8_t sq2_period_low;    /* Low byte of period for square wave 2. */
		uint8_t sq2_period_high;   /* High byte of period and length counter value for square wave 2. */

		uint8_t tri_counter;       /* Triangle wave linear counter. */
		uint8_t unused1;           /* Unused value apparently stored in APU section of CPU. */
		uint8_t tri_period_low;    /* Low byte of period for triangle wave. */
		uint8_t tri_period_high;   /* High byte of period and length counter value for triangle wave. */

		uint8_t noise_volume;      /* Volume for noise generator. */
		uint8_t unused2;           /* Unused value apparently stored in APU section of CPU. */
		uint8_t noise_period_low;  /* Period and waveform shape for noise generator. */
		uint8_t noise_period_high; /* Length counter value for noise generator. */

		uint8_t dmc_frequency;     /* Play mode and frequency for DMC samples. */
		uint8_t dmc_raw;           /* 7-bit DAC. */
		uint8_t dmc_start;         /* Start of DMC waveform is at address $C000 + $40*$xx */
		uint8_t dmc_length;        /* Length of DMC waveform is $10*$xx + 1 bytes (128*$xx + 8 samples). */

		uint8_t status_register;   /* Sound channels enable and status */

		uint8_t frame_counter;     /* Frame counter control. */

		std::vector<char> memory;
};

#endif /* __APU_HPP__ */