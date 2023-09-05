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

#ifndef __MAPPER_MMC1_HPP__
#define __MAPPER_MMC1_HPP__

#include <cstdint>

#include "../NESSystem.hpp"
#include "Mapper.hpp"

/**
 * MMC1 Memory Map:
 *
 * CPU $6000-$7FFF: 8 KB PRG RAM bank, fixed on all boards but SOROM and SXROM
 * CPU $8000-$BFFF: 16 KB PRG ROM bank, either switchable or fixed to the first bank
 * CPU $C000-$FFFF: 16 KB PRG ROM bank, either fixed to the last bank or switchable
 *
 * PPU $0000-$0FFF: 4 KB switchable CHR bank
 * PPU $1000-$1FFF: 4 KB switchable CHR bank
 *
 * PRG ROM max is 512k, minimum bank size 16k, leading to 32 banks possible.
 * CHR ROM max is 128k, minimum bank size 4k, leading to 32 banks possible.
 *
 */
class MapperMMC1 : public Mapper {

	public:
		MapperMMC1(NESSystem* nes_system, uint8_t number, uint8_t variant);
		~MapperMMC1();

		void Initialize(Cartridge* cartridge);
		void Shutdown();

		void ApplyState();

		uint8_t ReadCPU(uint16_t address);
		void WriteCPU(uint16_t address, uint8_t value);

		uint8_t ReadPPU(uint16_t address);
		void WritePPU(uint16_t address, uint8_t value);

	private:
		NESSystem* nes_system;

		rom_bank_t* prg_ram_bank;
		rom_bank_t* prg_rom_bank_1;
		rom_bank_t* prg_rom_bank_2;

		rom_bank_t* chr_rom_bank_1;
		rom_bank_t* chr_rom_bank_2;

		uint8_t control_register;
		uint8_t chr_bank0_register;
		uint8_t chr_bank1_register;
		uint8_t prg_bank_register;

		uint8_t register_select;
		uint8_t shift_register;
		uint8_t write_counter;

		enum Variant {
			
		};
};

#endif /* __MAPPER_MMC1_HPP__ */