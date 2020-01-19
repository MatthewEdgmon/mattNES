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

#include <bitset>
#include <fstream>
#include <iostream>
#include <string>

#include "../HexOutput.hpp"

#include "Mappers\Mapper.hpp"
#include "Mappers\MapperMMC1.hpp"
#include "Mappers\MapperMMC5.hpp"
#include "Mappers\MapperNROM.hpp"
#include "iNESHeader.hpp"
#include "Cartridge.hpp"
#include "UNIFHeader.hpp"

Cartridge::Cartridge(NESSystem* nes_system) : nes_system(nes_system) {
	prg_rom_size = 0;
	prg_ram_size = 0;
	chr_rom_size = 0;
	chr_ram_size = 0;
	mapper = 0;
	loaded = false;
}

Cartridge::~Cartridge() {
	prg_rom_size = 0;
	prg_ram_size = 0;
	chr_rom_size = 0;
	chr_ram_size = 0;
	mapper = 0;
	loaded = false;
}

void Cartridge::Initialize() {
	header = new iNES_header_t;
	header_unif = new unif_header_t;
}

void Cartridge::Shutdown() {
	delete header;
	delete header_unif;
}

void Cartridge::OpenFile(std::string file_name) {
	this->file_name = file_name;

	std::ifstream file(file_name, std::ios::ate | std::ios::binary);

	if(!file.is_open()) {
		std::cout << "Failed to open file \"" << file_name << "\"." << std::endl;
		abort();
	}

	size_t file_size = file.tellg();
	file_memory.resize(file_size);

	file.seekg(0);
	file.read(file_memory.data(), file_size);
	file.close();

	std::cout << "Successfully opened \"" << file_name << "\" (" << file_size << " bytes)..." << std::endl;

	/* Test for iNES header. */
	if(file_memory.at(0) == 'N' || file_memory.at(1) == 'E' || file_memory.at(2) == 'S' || file_memory.at(3) == 0x1A) {

		/* Grab 7th byte to determine iNES 1.0/2.0 */
		std::bitset<8> byte7(file_memory.at(7));

		if(byte7.test(2) == 1 && byte7.test(3) == 1) {
			std::cout << "iNES 2.0 ROM." << std::endl;
			header->is_iNES2 = true;
			header_type = HEADER_TYPE_INES2;
			OpeniNES2();
			loaded = true;
			return;
		} else {
			std::cout << "iNES 1.0 ROM." << std::endl;
			header->is_iNES2 = false;
			header_type = HEADER_TYPE_INES;
			OpeniNES();
			loaded = true;
			return;
		}
	}
	
	/* Test for UNIF header. */
	if(file_memory.at(0) == 'U' || file_memory.at(1) == 'N' || file_memory.at(2) == 'I' || file_memory.at(3) == 'F') {
		header_type = HEADER_TYPE_UNIF;
		OpenUNIF();
		loaded = true;
		return;
	}

	std::cout << "Opened non-ROM file." << std::endl;
	loaded = false;
	return;
}

void Cartridge::OpeniNES() {
	if(header->is_iNES2) {
		return;
	}

	/* Grab data from memory. */
	header->prg_rom_size = file_memory.at(4);
	header->chr_rom_size = file_memory.at(5);
	header->flags1       = file_memory.at(6);
	header->flags2       = file_memory.at(7);
	header->prg_ram_size = file_memory.at(8);
	header->flags3       = file_memory.at(9);
	header->flags4       = file_memory.at(10);

	/* If CHR ROM size was zero, the game uses CHR RAM instead. */
	if(header->chr_rom_size == 0) {
		header->chr_ram_size = 8;
	} else {
		header->chr_ram_size = 0;
	}

	/* Few dumpers/emulators actually supported PRG RAM size, so if it's zero, assume 8KB for compatability. */
	if(header->prg_ram_size == 0) {
		header->prg_ram_size = 1;
	}

	uint16_t mapper_number = (header->flags1 >> 4) + (header->flags2 & 0xF0);

	std::cout << "PRG ROM Size: " << +header->prg_rom_size * 16 << " KB" << std::endl;
	std::cout << "CHR ROM Size: " << +header->chr_rom_size * 8  << " KB" << std::endl;
	std::cout << "PRG RAM Size: " << +header->prg_ram_size * 8  << " KB" << std::endl;
	std::cout << "CHR RAM Size: " << +header->chr_ram_size * 8  << " KB" << std::endl;

	prg_rom_size = header->prg_rom_size * 0x4000;
	prg_ram_size = header->prg_ram_size * 0x2000;
	chr_rom_size = header->chr_rom_size * 0x2000;
	chr_ram_size = header->chr_ram_size * 0x2000;

	/* Convert flags to bitsets for parsing. */
	std::bitset<8> flags_6(header->flags1);
	std::bitset<8> flags_7(header->flags2);
	std::bitset<8> flags_9(header->flags3);
	std::bitset<8> flags_10(header->flags4);

	if(flags_6.test(0) == 0) {
		std::cout << "ROM uses horizontal mirroring." << std::endl;
	} else {
		std::cout << "ROM uses vertical mirroring." << std::endl;
	}

	if(flags_6.test(1) == 1) {
		std::cout << "ROM uses battery backed PRG RAM (0x6000 - 0x7FFF)." << std::endl;
	}

	if(flags_6.test(2) == 1) {
		std::cout << "ROM uses 512-byte trainer at (0x7000 - 0x71FF)." << std::endl;
		header->has_trainer = true;
	} else {
		header->has_trainer = false;
	}

	if(flags_6.test(3) == 1) {
		std::cout << "ROM uses four screen VRAM." << std::endl;
	}

	if(flags_7.test(0) == 1) {
		std::cout << "ROM is for the VS Unisystem." << std::endl;
	}

	if(flags_7.test(1) == 1) {
		std::cout << "ROM is for the Playchoice-10 (8KB of Hint Screen data is after CHR data)." << std::endl;
	}

	mapper = DetermineMapper(header);
	mapper->Initialize(this);

	std::cout << "ROM is using mapper \"" << mapper->GetName() << "\" (Number: " << mapper->GetNumber() << " Variant: " << mapper->GetVariant() << " )." << std::endl;
}

void Cartridge::OpeniNES2() {
	if(!header->is_iNES2) {
		return;
	}

	mapper = DetermineMapper(header);
	mapper->Initialize(this);

	std::cout << "ROM is using mapper \"" << mapper->GetName() << "\" (Number: " << mapper->GetNumber() << " Variant: " << mapper->GetVariant() << " )." << std::endl;
}

void Cartridge::OpenUNIF() {
	header_unif = new unif_header_t;

	mapper = DetermineMapper(header_unif);
	mapper->Initialize(this);

	std::cout << "ROM is using mapper \"" << mapper->GetName() << "\" (Number: " << mapper->GetNumber() << " Variant: " << mapper->GetVariant() << " )." << std::endl;
}

Mapper* Cartridge::DetermineMapper(iNES_header_t* header) {

	uint8_t mapper_number = (header->flags1 >> 4) + (header->flags2 & 0xF0);
	uint8_t mapper_variant = 0;

	if(header->is_iNES2) {
		mapper_variant = header->mapper_variant;
	}

	switch(mapper_number) {
		case 000:
			return new MapperNROM(nes_system, mapper_number, mapper_variant);
		case 001:
			return new MapperMMC1(nes_system, mapper_number, mapper_variant);
		case 005:
			return new MapperMMC5(nes_system, mapper_number, mapper_variant);
		default:
			std::cout << "WARNING: ROM is using unsupported mapper number" << mapper_number << "." << std::endl;
			return new MapperNROM(nes_system, mapper_number, mapper_variant);
	}
}

Mapper* Cartridge::DetermineMapper(unif_header_t* header) {
	std::cout << "WARNING: UNIF Loading is unsupported." << std::endl;
	return new MapperNROM(nes_system, 0, 0);
}

uint32_t Cartridge::GetHeaderOffset() {

	uint32_t offset = 0;

	if(header->has_trainer) {
		offset += 512;
	}

	switch(header_type) {
		case HEADER_TYPE_INES:
			offset += 16;
			break;
		case HEADER_TYPE_INES2:
			offset += 16;
			break;
		case HEADER_TYPE_UNIF:
			offset += 32;
			break;
		default:
			std::cout << "Tried to get header offset for a header type not supported. Aborting." << std::endl;
			offset += 0;
			break;
	}

	return offset;
}