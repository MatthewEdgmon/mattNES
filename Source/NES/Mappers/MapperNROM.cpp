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

#include <iostream>
#include <map>

#include "../../HexOutput.hpp"

#include "../Cartridge.hpp"
#include "Mapper.hpp"
#include "MapperNROM.hpp"

MapperNROM::MapperNROM(NESSystem* nes_system, uint8_t number, uint8_t variant) : nes_system(nes_system) {
	mapper_name = "NROM";
	mapper_number = number;
	mapper_variant = variant;
}

MapperNROM::~MapperNROM() {

}

void MapperNROM::Initialize(Cartridge* cartridge) {
	this->cartridge = cartridge;

	prg_rom_firstKB = DefineBank(0x8000, 0xBFFF, PRG_ROM, true);

	for(size_t i = 0; i < 0x3FFF; i++) {
		prg_rom_firstKB->data.at(i) = cartridge->GetFileMemory().at(i + cartridge->GetHeaderOffset());
	}
	
	memory_map_cpu[0x8000] = prg_rom_firstKB;

	/* Check if second KB, if not mirror first kb.*/
	if(cartridge->GetHeader()->prg_rom_size == 2) {
		prg_rom_secondKB = DefineBank(0xC000, 0xFFFF, PRG_ROM, true);

		for(size_t i = 0; i < 0x3FFF; i++) {
			prg_rom_secondKB->data.at(i) = cartridge->GetFileMemory().at(i + 0x4000 + cartridge->GetHeaderOffset());
		}

		memory_map_cpu[0xC000] = prg_rom_secondKB;
	} else {
		memory_map_cpu[0xC000] = prg_rom_firstKB;
	}

	prg_ram = DefineBank(0x6000, 0x7FFF, PRG_RAM, false);

	/* Check if Family BASIC ROM, and setup PRG RAM if so. */
	if(cartridge->GetHeader()->prg_ram_size == 1){
		prg_ram->mapped = true;
		memory_map_cpu[0x6000] = prg_ram;
	}

	chr_rom = DefineBank(0x0000, 0x1FFF, CHR_ROM, true);

	/* Skip over PRG ROM sections. */
	for(size_t i = 0; i < 0x1FFF; i++) {
         chr_rom->data.at(i) = cartridge->GetFileMemory().at(i + (cartridge->GetHeader()->prg_rom_size * 0x4000) + cartridge->GetHeaderOffset());
	}

	memory_map_ppu[0x0000] = chr_rom;
}

void MapperNROM::Shutdown() {
	DeleteBank(prg_rom_firstKB);
	DeleteBank(prg_rom_secondKB);
	DeleteBank(prg_ram);
	DeleteBank(chr_rom);
}

void MapperNROM::ApplyState() {
	// Do nothing, NROM is mapped completely at bootup.
}

uint8_t MapperNROM::ReadCPU(uint16_t address) {

	if(address >= 0x6000 && address <= 0x7FFF) {
		if(prg_ram->mapped) {
			return memory_map_cpu[0x6000]->data.at((address - 0x6000));
		}
	}

	if(address >= 0x8000 && address <= 0xBFFF) {
		return memory_map_cpu[0x8000]->data.at((address - 0x8000));
	}

	if(address >= 0xC000 && address <= 0xFFFF) {
		return memory_map_cpu[0xC000]->data.at((address - 0xC000));
	}

	std::cout << "Unknown ROM read from " << HEX4(address) << std::endl;
	return 0x00;
}

void MapperNROM::WriteCPU(uint16_t address, uint8_t value) {

	if(address >= 0x6000 && address <= 0x7FFF) {
		if(prg_ram->mapped) {
			memory_map_cpu[0x6000]->data.at((address - 0x6000)) = value;
		}
	}

	std::cout << "Unknown ROM write " << HEX2(value) << " to " << HEX4(address) << std::endl;
	return;
}

uint8_t MapperNROM::ReadPPU(uint16_t address) {

	if(address >= 0x0000 && address <= 0x1FFF) {
		return memory_map_ppu[0x0000]->data.at((address));
	}

	std::cout << "Unknown ROM read from " << HEX4(address) << std::endl;
	return 0x00;
}

void MapperNROM::WritePPU(uint16_t address, uint8_t value) {
	std::cout << "Unknown ROM write " << HEX2(value) << " to " << HEX4(address) << std::endl;
	return;
}