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

		float GetOutputState();

	private:
		uint16_t GetPulse1Output();
		uint16_t GetPulse2Output();
		uint16_t GetTriangleOutput();
		uint16_t GetNoiseOutput();
		uint16_t GetDMCOutput();

		NESSystem* nes_system;

		uint8_t sq1_volume { 0 };        /* Duty and volume for square wave 1. */
		uint8_t sq1_sweep { 0 };         /* Sweep control register for square wave 1. */
		uint8_t sq1_period_low { 0 };    /* Low byte of period for square wave 1. */
		uint8_t sq1_period_high { 0 };   /* High byte of period and length counter value for square wave 1. */

		uint8_t sq2_volume { 0 };        /* Duty and volume for square wave 2. */
		uint8_t sq2_sweep { 0 };         /* Sweep control register for square wave 2. */
		uint8_t sq2_period_low { 0 };    /* Low byte of period for square wave 2. */
		uint8_t sq2_period_high { 0 };   /* High byte of period and length counter value for square wave 2. */

		uint8_t tri_counter { 0 };       /* Triangle wave linear counter. */
		uint8_t unused1 { 0 };           /* Unused value apparently stored in APU section of CPU. */
		uint8_t tri_period_low { 0 };    /* Low byte of period for triangle wave. */
		uint8_t tri_period_high { 0 };   /* High byte of period and length counter value for triangle wave. */

		uint8_t noise_volume { 0 };      /* Volume for noise generator. */
		uint8_t unused2 { 0 };           /* Unused value apparently stored in APU section of CPU. */
		uint8_t noise_period_low { 0 };  /* Period and waveform shape for noise generator. */
		uint8_t noise_period_high { 0 }; /* Length counter value for noise generator. */

		uint8_t dmc_frequency { 0 };     /* Play mode and frequency for DMC samples. */
		uint8_t dmc_raw { 0 };           /* 7-bit DAC. */
		uint8_t dmc_start { 0 };         /* Start of DMC waveform is at address $C000 + $40*$xx */
		uint8_t dmc_length { 0 };        /* Length of DMC waveform is $10*$xx + 1 bytes (128*$xx + 8 samples). */

		uint8_t status_register { 0 };   /* Sound channels enable and status */

		uint8_t frame_counter { 0 };     /* Frame counter control. */

		float pulse_table[31];
		float tri_noise_dmc_table[203];

		std::vector<uint8_t> memory { 0 };
};

#endif /* __APU_HPP__ */