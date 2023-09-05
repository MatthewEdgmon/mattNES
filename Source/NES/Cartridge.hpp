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

#ifndef __CARTRIDGE_HPP__
#define __CARTRIDGE_HPP__

#include "./Mappers/Mapper.hpp"
#include "iNESHeader.hpp"
#include "UNIFHeader.hpp"

#include <string>
#include <vector>

class NESSystem;

class Cartridge {

	public:
		Cartridge(NESSystem* nes_system);
		~Cartridge();

		void Initialize();
		void Shutdown();

		void OpenFile(std::string file_name);

		uint32_t GetHeaderOffset();

		Mapper* GetMapper()               { return mapper; };
		iNES_header_t* GetHeader()        { return header; };
		std::vector<char> GetFileMemory() { return file_memory; };

		uint32_t GetPRGROMSize() { return prg_rom_size; };
		uint32_t GetPRGRAMSize() { return prg_ram_size; };
		uint32_t GetCHRROMSize() { return chr_rom_size; };
		uint32_t GetCHRRAMSize() { return chr_ram_size; };

		bool IsLoaded() { return loaded; };

	private:
		NESSystem* nes_system;

		void OpeniNES();
		void OpeniNES2();
		void OpenUNIF();

		typedef enum HeaderType {
			HEADER_TYPE_INES,
			HEADER_TYPE_INES2,
			HEADER_TYPE_UNIF
		} header_type_t;

		Mapper* DetermineMapper(iNES_header_t* header);
		Mapper* DetermineMapper(unif_header_t* header);

		Mapper* mapper;
		header_type_t header_type;
		iNES_header_t* header;
		unif_header* header_unif;

		std::string file_name;
		std::vector<char> file_memory;

		uint32_t prg_rom_size;
		uint32_t prg_ram_size;
		uint32_t chr_rom_size;
		uint32_t chr_ram_size;

		bool loaded;
};

#endif /* __CARTRIDGE_HPP__ */