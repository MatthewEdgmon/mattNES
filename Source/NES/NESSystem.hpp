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

#ifndef __NES_SYSTEM_HPP__
#define __NES_SYSTEM_HPP__

#include <string>

class ControllerIO;
class Cartridge;
class APU;
class CPU;
class PPU;

class NESSystem {

	public:
		typedef enum CPUEmulationMode {
			RP2A03,
			RP2A03G,
			RP2A07G,	
			DENDY
		} cpu_emulation_mode_t;

		typedef enum PPUEmulationMode {
			RP2C02, /* North America NTSC */
			RP2C02A,
			RP2C03,
			RP2C03B,
			RP2C04,
			RP2C04_0001,
			RP2C04_0002,
			RP2C04_0003,
			RP2C04_0004,
			RP2C05,
			RP2C07 /* PAL */
		} ppu_emulation_mode_t;

		typedef enum RegionEmulationMode {
			NTSC,
			PAL
		} region_emulation_mode_t;

	public:
		NESSystem(cpu_emulation_mode_t cpu_type, ppu_emulation_mode_t ppu_type, region_emulation_mode_t region);
		~NESSystem();

		void Initialize(std::string rom_file_name);
		void Shutdown();
		void Reset(bool hard);
		void Frame();

		void DumpTestInfo();

		cpu_emulation_mode_t    GetCPUModel() { return cpu_emulation_mode;    };
		ppu_emulation_mode_t    GetPPUModel() { return ppu_emulation_mode;    };
		region_emulation_mode_t GetRegion()   { return region_emulation_mode; };

		ControllerIO* GetControllerIO() { return controller_io; }
		Cartridge* GetCartridge() { return cartridge; }
		APU* GetAPU() { return apu; }
		CPU* GetCPU() { return cpu; }
		PPU* GetPPU() { return ppu; }

		uint8_t GetFloatingBus() { return floating_bus_value; }
		void SetFloatingBus(uint8_t value) { floating_bus_value = value; }

	private:
		cpu_emulation_mode_t    cpu_emulation_mode;
		ppu_emulation_mode_t    ppu_emulation_mode;
		region_emulation_mode_t region_emulation_mode;

		ControllerIO* controller_io;
		Cartridge* cartridge;
		APU* apu;
		CPU* cpu;
		PPU* ppu;

		/* Value on the data busses between CPU, APU, and PPU to emulate bus conflict and floating bus behaviour. */
		uint8_t floating_bus_value;
		// TODO: Depending on which chip had the last cycle, floating capacitance on the bus will be different. Use these two values to emulate.
		uint8_t floating_bus_value_cpu;
		uint8_t floating_bus_value_ppu;
};

#endif /* __NES_SYSTEM_HPP__ */