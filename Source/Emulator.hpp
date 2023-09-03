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

#ifndef __EMULATOR_HPP__
#define __EMULATOR_HPP__

#include <fstream>
#include <memory>
#include <string>
#include <utility>

#include <SDL.h>

#include "NES/ControllerIO.hpp"

extern "C" {
	void EmulatorAudioCallback(void* userdata, Uint8 stream, int length);
}

class NESSystem;

class Emulator {

	public:
		Emulator();
		~Emulator();

		int Main(int argc, char** argv);

		void Initialize();
		void Loop();
		void Shutdown();

		void Render();

		void HandleInputDown();
		void HandleInputUp();

		void HandleCommandLine();

		void AudioCallback(std::uint8_t* stream, int length);

	private:
		SDL_Window*       sdl_window { nullptr };
		SDL_Renderer*     sdl_renderer { nullptr };
		SDL_RendererInfo  sdl_renderer_info { 0 };
		SDL_Texture*      sdl_texture { nullptr };
		SDL_AudioDeviceID sdl_audio_device_id { 0 };

		SDL_Thread*		  sdl_emulation_thread { nullptr };

		SDL_Event         sdl_event { 0 };

		/* The system being emulated. */
		std::unique_ptr<NESSystem> nes_system;

		/* Log file for the emulator. */
		std::ofstream log_file;

		/* File name of the ROM being run. */
		std::string file_name;

		/* Return value from emulation thread. */
		int sdl_emulation_thread_value { 0 };

		bool display_framerate { false };
		bool display_video { true };
		bool emulation_paused { false };
		bool disassemble_cpu { false };
		bool log_to_file { false };
		bool is_fully_initialized{ false };
		bool is_running { false };

		/* Stored arguments. */
		int stored_argc { 0 };
		char** stored_argv { nullptr};
};

#endif /* __EMULATOR_HPP__ */