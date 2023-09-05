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

#ifndef __MAPPER_NROM_HPP__
#define __MAPPER_NROM_HPP__

#include <string>

#include "Mapper.hpp"

/**
 * NROM Memory Map:
 *
 * CPU 0x6000 - 0x7FFF: Family Basic only: PRG RAM, mirrored to fit entire 8 KiB window, write protectable.
 * CPU 0x8000 - 0xBFFF: First 16 kB of PRG-ROM.
 * CPU 0xC000 - 0xFFFF: Optional second 16 kB of PRG-ROM (NROM-256) or mirror of 0x8000 - 0xBFFF (NROM-128).
 *
 * PPU 0x0000 - 0x1FFF: First and only 8kB CHR-ROM
 */
class MapperNROM : public Mapper {

	public:
		MapperNROM(NESSystem* nes_system, uint8_t number, uint8_t variant);
		~MapperNROM();

		void Initialize(Cartridge* cartridge);
		void Shutdown();

		void ApplyState();

		uint8_t ReadCPU(uint16_t address);
		void WriteCPU(uint16_t address, uint8_t value);

		uint8_t ReadPPU(uint16_t address);
		void WritePPU(uint16_t address, uint8_t value);

	private:
		NESSystem* nes_system;

		rom_bank_t* prg_rom_firstKB;
		rom_bank_t* prg_rom_secondKB;
		rom_bank_t* prg_ram;
		rom_bank_t* chr_rom;

		enum Variant {
			NROM_128,
			NROM_256,
			HVC
		};
};

#endif /* __MAPPER_NROM_HPP__ */