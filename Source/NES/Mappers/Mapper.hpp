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

#ifndef __MAPPER_HPP__
#define __MAPPER_HPP__

#include <cstdint>
#include <map>
#include <string>
#include <vector>

typedef enum class bank_type {
	PRG_ROM,
	PRG_RAM,
	CHR_ROM,
	CHR_RAM
} bank_type_t;

typedef struct rom_bank {
	bank_type type { bank_type::PRG_ROM };
	uint16_t rom_address_start { 0 };
	uint16_t rom_address_end { 0 };
	uint16_t size { 0 };
	std::vector<uint8_t> data { 0 };
	bool mapped { false };
} rom_bank_t;

class Cartridge;

class Mapper {

	public:
		virtual void Initialize(Cartridge* cartride) =0;
		virtual void Shutdown() =0;
		
		virtual void ApplyState() =0;

		virtual uint8_t ReadCPU(uint16_t address) =0;
		virtual void WriteCPU(uint16_t address, uint8_t value) =0;

		virtual uint8_t ReadPPU(uint16_t address) =0;
		virtual void WritePPU(uint16_t address, uint8_t value) =0;

		rom_bank_t* DefineBank(uint16_t map_address_start, uint16_t map_address_end, bank_type_t type, bool mapped) {
			rom_bank_t* new_bank = new rom_bank_t;
			new_bank->mapped = mapped;
			new_bank->rom_address_start = map_address_start;
			new_bank->rom_address_end = map_address_end;
			new_bank->size = (new_bank->rom_address_end - new_bank->rom_address_start) + 1;
			new_bank->type = type;
			new_bank->data.resize(new_bank->size);
			return new_bank;
		}

		void DeleteBank(rom_bank_t* bank) {
			if(bank) {
				delete bank;
			}
		}

		std::string GetName() { return mapper_name; };
		uint8_t GetNumber() { return mapper_number; };
		uint8_t GetVariant() { return mapper_variant; };

	protected:
		Cartridge* cartridge;

		std::map<uint16_t, rom_bank_t*> memory_map_cpu;
		std::map<uint16_t, rom_bank_t*> memory_map_ppu;

		std::string mapper_name;
		uint8_t mapper_number;
		uint8_t mapper_variant;
};

#endif /* __MAPPER_HPP__ */