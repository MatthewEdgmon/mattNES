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

#include "../../BitOps.hpp"
#include "../../HexOutput.hpp"

#include "../Cartridge.hpp"
#include "../NESSystem.hpp"
#include "../PPU.hpp"
#include "Mapper.hpp"
#include "MapperMMC1.hpp"

MapperMMC1::MapperMMC1(NESSystem* nes_system, uint8_t number, uint8_t variant) : nes_system(nes_system) {
	mapper_number = number;
	mapper_variant = variant;

	switch(mapper_variant) {
		case 0:
			mapper_name = "MMC1";
			break;
		case 1:
		case 2:
		case 4:
		case 5:
		default:
			std::cout << "Invalid MMC1 mapper variant, assuming normal." << std::endl;
			mapper_name = "MMC1";
			break;
	}

	control_register = 0;
	chr_bank0_register = 0;
	chr_bank1_register = 0;
	prg_bank_register = 0;

	register_select = 0;
	shift_register = 0;
	write_counter = 0;
}

MapperMMC1::~MapperMMC1() {
	mapper_name = "";
	mapper_number = 0;
	mapper_variant = 0;

	control_register = 0;
	chr_bank0_register = 0;
	chr_bank1_register = 0;
	prg_bank_register = 0;

	register_select = 0;
	shift_register = 0;
	write_counter = 0;
}

void MapperMMC1::Initialize(Cartridge* cartridge) {
	this->cartridge = cartridge;

	/* Start with 16k PRG swapping, on 0x8000 - 0xBFFF. */
	control_register = 0x0C;

	prg_ram_bank = DefineBank(0x6000, 0x7FFF, bank_type::PRG_RAM, true);
	prg_rom_bank_1 = DefineBank(0x8000, 0xBFFF, bank_type::PRG_ROM, true);
	prg_rom_bank_2 = DefineBank(0xC000, 0xFFFF, bank_type::PRG_ROM, true);

	chr_rom_bank_1 = DefineBank(0x0000, 0x0FFF, bank_type::CHR_ROM, true);
	chr_rom_bank_2 = DefineBank(0x1000, 0x1FFF, bank_type::CHR_ROM, true);

	memory_map_cpu[prg_ram_bank->rom_address_start] = prg_ram_bank;
	memory_map_cpu[prg_rom_bank_1->rom_address_start] = prg_rom_bank_1;
	memory_map_cpu[prg_rom_bank_2->rom_address_start] = prg_rom_bank_2;

	memory_map_ppu[chr_rom_bank_1->rom_address_start] = chr_rom_bank_1;
	memory_map_ppu[chr_rom_bank_2->rom_address_start] = chr_rom_bank_2;

	/* Initialize RAM to zeros. */
	for(size_t i = 0; i < prg_ram_bank->size; i++) {
		prg_ram_bank->data[i] = 0x0;
	}

	/* First ROM bank is at the top. */
	for(size_t i = 0; i < prg_rom_bank_1->size; i++) {
		prg_rom_bank_1->data[i] = cartridge->GetFileMemory()[i + cartridge->GetHeaderOffset()];
	}

	/* Second ROM bank is 0x4000 bytes after the first. */
	for(size_t i = 0; i < prg_rom_bank_2->size; i++) {
		prg_rom_bank_2->data[i] = cartridge->GetFileMemory()[i + 0x4000 + cartridge->GetHeaderOffset()];
	}

	/* First CHR ROM bank is 0x8000 bytes after PRG ROM. */
	for(size_t i = 0; i < chr_rom_bank_1->size; i++) {
		chr_rom_bank_1->data[i] = cartridge->GetFileMemory()[i + 0x4000 + 0x4000 + cartridge->GetHeaderOffset()];
	}

	for(size_t i = 0; i < chr_rom_bank_2->size; i++) {
		chr_rom_bank_2->data[i] = cartridge->GetFileMemory()[i + 0x4000 + 0x4000 + 0x4000 + cartridge->GetHeaderOffset()];
	}
}

void MapperMMC1::Shutdown() {
	DeleteBank(prg_ram_bank);
	DeleteBank(prg_rom_bank_1);
	DeleteBank(prg_rom_bank_2);
	DeleteBank(chr_rom_bank_1);
	DeleteBank(chr_rom_bank_2);
}

void MapperMMC1::ApplyState() {

	/* Control register mirroring. */
	switch(control_register & 0x03) {
		case 0: /* One-screen lower bank. */
			nes_system->GetPPU()->SetMirroringMode();
			break;
		case 1: /* One-screen upper bank. */
			nes_system->GetPPU()->SetMirroringMode();
			break;
		case 2: /* Vertical */
			nes_system->GetPPU()->SetMirroringMode();
			break;
		case 3: /* Horizontal */
			nes_system->GetPPU()->SetMirroringMode();
		default:
			/* Unreachable. */
			break;
	}

	/* Control register PRG ROM banking mode. */
	switch((control_register & 0x0C) >> 2) {
		case 0:
			break;
		case 1:
			break;
		case 2:
			break;
		case 3: /* Fix last bank at 0xC000 and switch 16KB bank at 0x8000 */
		default:
			/* Unreachable. */
			break;
	}

	/* Control register CHR ROM banking mode. */
	if(BitCheck(control_register, 4)) {
		/* Switch two seperate 4KB banks. */
	} else {
		/* Switch 8KB at a time. */
	}

	/* PRG bank register. */

	/* PRG bank PRG RAM enable. */
	if(BitCheck(prg_bank_register, 4)) {
		/* PRG RAM disabled. */
		prg_ram_bank->mapped = false;
	} else {
		/* PRG RAM enabled. */
		prg_ram_bank->mapped = true;
	}

	if(mapper_name == "MMC1A") {
		/* PRG RAM is always enabled on this variant. */
		prg_ram_bank->mapped = true;
	}
}

uint8_t MapperMMC1::ReadCPU(uint16_t address) {

	/* Read from PRG RAM */
	if(address >= 0x6000 && address <= 0x7FFF) {
		if(prg_ram_bank->mapped) {
			return memory_map_cpu[0x6000]->data[address - 0x6000];
		}
	} 

	if(address >= 0x8000 && address <= 0xBFFF) {
		return memory_map_cpu[0x8000]->data[address - 0x8000];
	}

	if(address >= 0xC000 && address <= 0xFFFF) {
		return memory_map_cpu[0xC000]->data[address - 0xC000];
	}

	std::cout << "Unknown ROM read from " << HEX(address) << std::endl;
	return 0x00;
}

void MapperMMC1::WriteCPU(uint16_t address, uint8_t value) {

	/* Write to PRG RAM */
	if(address >= 0x6000 && address <= 0x7FFF) {
		if(prg_ram_bank->mapped) {
			memory_map_cpu[0x6000]->data[address - 0x6000] = value;
			return;
		}
	} 

	/* The MMC1 serial port exists at any address between 0x8000 and 0xFFFF. */
	if(address >= 0x8000 || address <= 0xFFFF) {

		/* Any write to 0x8000 through 0xFFFF with value bit 7 set clears shift register and ORs control register with 0xC0. */
		if(BitCheck(value, 7)) {
			shift_register = 0;
			write_counter = 0;

			control_register |= 0xC0;

			ApplyState();
			return;
		}

		/* Count four writes with bit 7 clear. */
		if(!BitCheck(value, 7) && write_counter <= 4) {
			shift_register += BitCheck(value, 0);
			shift_register <<= 1;
			write_counter++;
			return;
		}

		/* Fifth write is where the magic happens. Bits 13 and 14 of address select the MMC1 register. */
		if(!BitCheck(value, 7) && write_counter == 5) {
			shift_register += BitCheck(value, 0);
			write_counter = 0;

			register_select  = BitCheck(address, 13);
			register_select += BitCheck(address, 14) << 1;

			ApplyState();
			return;
		}

		return;
	}

	std::cout << "Unknown ROM write " << HEX2(value) << " to " << HEX4(address) << std::endl;
	return;
}

uint8_t MapperMMC1::ReadPPU(uint16_t address) {

	if(address >= 0x0000 && address <= 0x0FFF) {
		return memory_map_ppu[0x0000]->data[address];
	}

	if(address >= 0x1000 && address <= 0x1FFF) {
		return memory_map_ppu[0x1000]->data[address];
	}

	std::cout << "Unknown ROM read from " << HEX4(address) << std::endl;
	return 0x00;
}

void MapperMMC1::WritePPU(uint16_t address, uint8_t value) {
	std::cout << "Unknown PPU write " << HEX2(value) << " to " << HEX4(address) << std::endl;
	return;
}