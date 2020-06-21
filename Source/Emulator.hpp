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
#include <string>

#include <SDL.h>

#include "NES/ControllerIO.hpp"

class NESSystem;

class Emulator {

	public:
		Emulator();
		~Emulator();

		int Main(int argc, char** argv);

		void Initialize();
		void Loop();
		void Shutdown();

		void HandleInputDown();
		void HandleInputUp();

		void HandleCommandLine();

	private:
		SDL_Window*       sdl_window;
		SDL_Renderer*     sdl_renderer;
		SDL_RendererInfo  sdl_renderer_info;
		SDL_Texture*      sdl_texture;
		SDL_AudioDeviceID sdl_audio_device_id;

		SDL_Event         sdl_event;

		NESSystem* nes_system;

		/* Log file for the emulator. */
		std::ofstream log_file;

		/* File name of the ROM being run. */
		std::string file_name;

		bool display_framerate;
		bool emulation_paused;
		bool single_step;
		bool dissamble_cpu;
		bool log_to_file;
		bool is_running;

		/* Stored arguments. */
		int stored_argc;
		char** stored_argv;
};

#endif /* __EMULATOR_HPP__ */