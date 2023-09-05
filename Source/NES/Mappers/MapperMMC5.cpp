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

#include <iostream>

#include "../../HexOutput.hpp"

#include "../NESSystem.hpp"
#include "MapperMMC5.hpp"

MapperMMC5::MapperMMC5(NESSystem* nes_system, uint8_t number, uint8_t variant) : nes_system(nes_system) {
	mapper_name = "MMC5";
	mapper_number = number;
	mapper_variant = variant;
}

MapperMMC5::~MapperMMC5() {

}

void MapperMMC5::Initialize(Cartridge* cartridge) {
	this->cartridge = cartridge;
}

void MapperMMC5::Shutdown() {

}

void MapperMMC5::ApplyState() {

}

uint8_t MapperMMC5::ReadCPU(uint16_t address) {
	std::cout << "Unknown ROM read from " << HEX(address) << std::endl;
	return 0x00;
}

void MapperMMC5::WriteCPU(uint16_t address, uint8_t value) {
	std::cout << "Unknown ROM write " << HEX(value) << " to " << HEX(address) << std::endl;
	return;
}

uint8_t MapperMMC5::ReadPPU(uint16_t address) {
	std::cout << "Unknown ROM read from " << HEX(address) << std::endl;
	return 0x00;
}

void MapperMMC5::WritePPU(uint16_t address, uint8_t value) {
	std::cout << "Unknown ROM write " << HEX(value) << " to " << HEX(address) << std::endl;
	return;
}