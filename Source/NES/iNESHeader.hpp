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

#ifndef __INES_HEADER_HPP__
#define __INES_HEADER_HPP__

#include <cstdint>

typedef struct iNES_header {
	uint16_t signature;      // Byte 0-3
	uint8_t  prg_rom_size;   // Byte 4
	uint8_t  chr_rom_size;   // Byte 5
	uint8_t  chr_ram_size;
	uint8_t  flags1;         // Byte 6
	uint8_t  flags2;         // Byte 7

	/* iNES 1.0 */
	uint8_t  prg_ram_size;   // Byte 8
	uint8_t  flags3;         // Byte 9
	uint8_t  flags4;         // Byte 10

	/* iNES 2.0 */
	uint8_t  mapper_variant;
	uint8_t  rom_size_bits_upper;
	uint8_t  ram_size;
	uint8_t  vram_size;
	uint8_t  tv_system;
	uint8_t  vs_system;
	uint8_t  misc_roms;
	uint8_t  reserved;

	bool is_iNES2;
	bool has_trainer;
} iNES_header_t;

#endif /* __INES_HEADER_HPP__ */