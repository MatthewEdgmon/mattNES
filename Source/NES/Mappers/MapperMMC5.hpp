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

#ifndef __MAPPER_MMC5_HPP__
#define __MAPPER_MMC5_HPP__

#include "Mapper.hpp"

class NESSystem;

class MapperMMC5 : public Mapper {

	public:
		MapperMMC5(NESSystem* nes_system, uint8_t number, uint8_t variant);
		~MapperMMC5();

		void Initialize(Cartridge* cartridge);
		void Shutdown();

		void ApplyState();

		uint8_t ReadCPU(uint16_t address);
		void WriteCPU(uint16_t address, uint8_t value);

		uint8_t ReadPPU(uint16_t address);
		void WritePPU(uint16_t address, uint8_t value);

	private:
		NESSystem* nes_system;

		enum PRGMode {
			PRG_MODE0,
			PRG_MODE1,
			PRG_MODE2,
			PRG_MODE3
		};

		enum CHRMode {
			CHR_MODE0,
			CHR_MODE1,
			CHR_MODE2,
			CHR_MODE3
		};

};

#endif /* __MAPPER_MMC5_HPP__ */