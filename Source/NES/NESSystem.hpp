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

#ifndef __NES_SYSTEM_HPP__
#define __NES_SYSTEM_HPP__

#include <memory>
#include <string>
#include <utility>

#define MEMORY_SIZE 65535

class ControllerIO;
class Cartridge;
class APU;
class CPU;
class PPU;

class NESSystem {

	public:
		typedef enum class CPUEmulationMode {
			RP2A03,
			RP2A03G,
			RP2A07G,	
			DENDY
		} cpu_emulation_mode_t;

		typedef enum class PPUEmulationMode {
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

		typedef enum class RegionEmulationMode {
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

		/* Emulated bus read/write. */
		uint8_t Read(uint16_t address);
		void Write(uint16_t address, uint8_t value);

		/* Read/write memory/devices without emulation side effects. */
		uint8_t Peek(uint16_t address);
		void Poke(uint16_t address, uint8_t value);

		std::string TestInfo();

		cpu_emulation_mode_t    GetCPUModel() { return cpu_emulation_mode;    };
		ppu_emulation_mode_t    GetPPUModel() { return ppu_emulation_mode;    };
		region_emulation_mode_t GetRegion()   { return region_emulation_mode; };

		ControllerIO* GetControllerIO() { return controller_io.get(); }
		Cartridge* GetCartridge() { return cartridge.get(); }
		APU* GetAPU() { return apu.get(); }
		CPU* GetCPU() { return cpu.get(); }
		PPU* GetPPU() { return ppu.get(); }

		uint64_t CycleCount() { return cycles; }

		uint8_t GetFloatingBus() { return floating_bus_value; }
		void SetFloatingBus(uint8_t value) { floating_bus_value = value; }

	private:
		void ObjectAttributeMemoryDMA(uint8_t value);

		cpu_emulation_mode_t    cpu_emulation_mode;
		ppu_emulation_mode_t    ppu_emulation_mode;
		region_emulation_mode_t region_emulation_mode;

		std::unique_ptr<ControllerIO> controller_io;
		std::unique_ptr<Cartridge> cartridge;
		std::unique_ptr<APU> apu;
		std::unique_ptr<CPU> cpu;
		std::unique_ptr<PPU> ppu;

		uint8_t* memory;

		/* Cycle count. */
		uint64_t cycles;

		/* Value on the data busses between CPU, APU, and PPU to emulate bus conflict and floating bus behaviour. */
		uint8_t floating_bus_value { 0 };
		// TODO: Depending on which chip had the last cycle, floating capacitance on the bus will be different. Use these two values to emulate.
		uint8_t floating_bus_value_cpu { 0 };
		uint8_t floating_bus_value_ppu { 0 };
};

#endif /* __NES_SYSTEM_HPP__ */