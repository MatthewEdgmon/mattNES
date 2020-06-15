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
#include <stdio.h>

#include <SDL.h>

#include "BitOps.hpp"
#include "NES/NESSystem.hpp"
#include "NES/ControllerIO.hpp"
#include "NES/CPU.hpp"
#include "NES/PPU.hpp"
#include "Emulator.hpp"

Emulator::Emulator() {

}

Emulator::~Emulator() {

}

int Emulator::Main(int argc, char** argv) {
	stored_argc = argc;
	stored_argv = argv;

	Initialize();
	Loop();
	Shutdown();

	return 0;
}

void Emulator::Initialize() {
	/* Parse arguments. */
	HandleCommandLine();

	/* Startup libraries.*/
    SDL_SetMainReady();
	SDL_Init(SDL_INIT_EVERYTHING);

	SDL_version sdl_compiled_version, sdl_linked_version;
	SDL_VERSION(&sdl_compiled_version);
	SDL_GetVersion(&sdl_linked_version);

	std::cout << "Using SDL2 engine." << '\n';
	std::cout << "Compiled with SDL2 version: " << sdl_compiled_version.major << "." << sdl_compiled_version.minor << "." << sdl_compiled_version.patch << '\n';
	std::cout << "Linked with SDL2 version:   " << sdl_linked_version.major << "." << sdl_linked_version.minor << "." << sdl_linked_version.patch << '\n';

	SDL_DisableScreenSaver();

	sdl_window = SDL_CreateWindow("mattNES", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, NES_SCREEN_WIDTH, NES_SCREEN_HEIGHT, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI);

	sdl_renderer = SDL_CreateRenderer(sdl_window, -1, SDL_RENDERER_ACCELERATED);

	SDL_GetRendererInfo(sdl_renderer, &sdl_renderer_info);

	std::cout << "Renderer Name: " << sdl_renderer_info.name << '\n';
	std::cout << "Renderer Texture Formats: ";

	for(size_t i = 0; i < sdl_renderer_info.num_texture_formats; i++) {
		std::cout << SDL_GetPixelFormatName(sdl_renderer_info.texture_formats[i]) << " ";
	}

	std::cout << '\n';

	sdl_texture = SDL_CreateTexture(sdl_renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, NES_SCREEN_WIDTH, NES_SCREEN_HEIGHT);

	SDL_AudioSpec want, have;

	SDL_zero(want);
	want.freq     = 44100;
	want.format   = AUDIO_S16SYS;
	want.channels = 1;
	want.samples  = 2048;
	want.callback = NULL;

	sdl_audio_device_id = SDL_OpenAudioDevice(0, 0, &want, &have, 0);

	// SDL Initializtion Finished Here.

	/* Create emulated system. */
	nes_system = new NESSystem(NESSystem::RP2A03, NESSystem::RP2C02, NESSystem::NTSC);

	/* Test ROMs */
	//nes_system->Initialize("Test/instr_test-v3/all_instrs.nes");
	nes_system->Initialize("Test/instr_test-v3/official_only.nes");
	//nes_system->Initialize("Test/instr_test-v3/rom_singles/01-implied.nes");
	//nes_system->Initialize("Test/instr_test-v3/rom_singles/02-immediate.nes");
	//nes_system->Initialize("Test/instr_test-v3/rom_singles/03-zero_page.nes");
	//nes_system->Initialize("Test/instr_test-v3/rom_singles/04-zp_xy.nes");
	//nes_system->Initialize("Test/instr_test-v3/rom_singles/05-absolute.nes");
	//nes_system->Initialize("Test/instr_test-v3/rom_singles/06-abs_xy.nes");
	//nes_system->Initialize("Test/instr_test-v3/rom_singles/07-ind_x.nes");
	//nes_system->Initialize("Test/instr_test-v3/rom_singles/08-ind_y.nes");
	//nes_system->Initialize("Test/instr_test-v3/rom_singles/09-branches.nes");
	//nes_system->Initialize("Test/instr_test-v3/rom_singles/10-stack.nes");
	//nes_system->Initialize("Test/instr_test-v3/rom_singles/11-jmp_jsr.nes");
	//nes_system->Initialize("Test/instr_test-v3/rom_singles/12-rts.nes");
	//nes_system->Initialize("Test/instr_test-v3/rom_singles/13-rti.nes");
	//nes_system->Initialize("Test/instr_test-v3/rom_singles/14-brk.nes");
	//nes_system->Initialize("Test/instr_test-v3/rom_singles/15-special.nes");

	//nes_system->Initialize("Test/instr_misc/instr_misc.nes");

	//nes_system->Initialize("Test/ppu_vbl_nmi/ppu_vbl_nmi.nes");
	//nes_system->Initialize("Test/nrom368/test1.nes");
	//nes_system->Initialize("Test/spritecans-2011/spritecans.nes");

	/* NROM Games */
	//nes_system->Initialize("ROMs/Baseball.nes");
	//nes_system->Initialize("ROMs/Donkey Kong.nes");
	//nes_system->Initialize("ROMs/Mario Bros..nes");
	//nes_system->Initialize("ROMs/Pinball.nes");

	/* Hard NROM Games */
	//nes_system->Initialize("ROMs/Super Mario Bros..nes");
}

void Emulator::Loop() {

	is_running = true;
	emulation_paused = true;
	display_framerate = false;

	char title_buffer[255];

	uint64_t frame_start = 0;
	uint64_t frame_end = 0;
	uint64_t perf_freq = 0;
	double frame_time = 0.0f;
	double frame_rate = 0.0f;

	while(is_running) {

		frame_start = SDL_GetPerformanceCounter();

		while(SDL_PollEvent(&sdl_event)) {

			switch(sdl_event.type) {
				case SDL_QUIT:
					is_running = false;
				case SDL_KEYDOWN:
					HandleInputDown();
					break;
				case SDL_KEYUP:
					HandleInputUp();
					break;
				case SDL_DROPFILE:
					// TODO: Support dragon drop files.
					break;
				default:
					break;
			}
		}

		if(emulation_paused == false) {
			nes_system->Frame();
		}

		//if(nes_system->GetCPU()->GetProgramCounter() == 0xE976) {
		//	nes_system->DumpTestInfo();
		//	emulation_paused = true;
		//}

		//SDL_RenderClear(sdl_renderer);

		//SDL_UpdateTexture(sdl_texture, NULL, nes_system->GetPPU()->GetVideoBuffer(), (NES_SCREEN_WIDTH * 4));

		//SDL_RenderCopy(sdl_renderer, sdl_texture, NULL, NULL);

		//SDL_RenderPresent(sdl_renderer);

		frame_end = SDL_GetPerformanceCounter();
		
		/* Only update the frametime counter every 100 ticks. */
		if(frame_end % 100 == 0) {
			perf_freq = SDL_GetPerformanceFrequency();
			frame_time = (frame_end - frame_start) / static_cast<double>(perf_freq);
			frame_rate = 1 / (frame_time * 1000.0);

			sprintf(title_buffer, "%f", (frame_time * 1000.0));
			//sprintf(title_buffer, "%f", frame_rate);
			SDL_SetWindowTitle(sdl_window, title_buffer);
		}

		if(display_framerate) {
			std::cout << "Frame Time: " << frame_time * 1000.0 << "ms" << '\n';
		}

		SDL_Delay(1);
	}
}

void Emulator::Shutdown() {
	/* Shutdown emulated system. */
	nes_system->Shutdown();
	delete nes_system;
	nes_system = nullptr;

	SDL_DestroyRenderer(sdl_renderer);

	SDL_DestroyWindow(sdl_window);

	/* Close libraries.*/
	SDL_Quit();
}

void Emulator::HandleInputDown() {

	if(nes_system->GetControllerIO()->IsControllerStrobeLatchHigh()) {
		switch(sdl_event.key.keysym.sym) {
			case SDLK_UP:    BitSet(nes_system->GetControllerIO()->GetControllerState()->controller_state_port1_D0, NES_CONTROLLER_UP); break;
			case SDLK_DOWN:  BitSet(nes_system->GetControllerIO()->GetControllerState()->controller_state_port1_D0, NES_CONTROLLER_DOWN); break;
			case SDLK_LEFT:  BitSet(nes_system->GetControllerIO()->GetControllerState()->controller_state_port1_D0, NES_CONTROLLER_LEFT); break;
			case SDLK_RIGHT: BitSet(nes_system->GetControllerIO()->GetControllerState()->controller_state_port1_D0, NES_CONTROLLER_RIGHT); break;
			case SDLK_z:     BitSet(nes_system->GetControllerIO()->GetControllerState()->controller_state_port1_D0, NES_CONTROLLER_SELECT); break;
			case SDLK_x:     BitSet(nes_system->GetControllerIO()->GetControllerState()->controller_state_port1_D0, NES_CONTROLLER_START); break;
			case SDLK_c:     BitSet(nes_system->GetControllerIO()->GetControllerState()->controller_state_port1_D0, NES_CONTROLLER_B); break;
			case SDLK_v:     BitSet(nes_system->GetControllerIO()->GetControllerState()->controller_state_port1_D0, NES_CONTROLLER_A); break;
			default: break;
		}
	}

	switch(sdl_event.key.keysym.sym) {
		case SDLK_RETURN:
			nes_system->Frame();
			break;
		case SDLK_d:
			nes_system->GetCPU()->ToggleDisassembly();
			break;
		case SDLK_e:
			emulation_paused = !emulation_paused;
			break;
		case SDLK_f:
			display_framerate = !display_framerate;
			break;
		case SDLK_t:
			nes_system->DumpTestInfo();
			break;
		default:
			break;
	}
}

void Emulator::HandleInputUp() {

	if(nes_system->GetControllerIO()->IsControllerStrobeLatchHigh()) {
		switch(sdl_event.key.keysym.sym) {
			case SDLK_UP:    BitClear(nes_system->GetControllerIO()->GetControllerState()->controller_state_port1_D0, NES_CONTROLLER_UP); break;
			case SDLK_DOWN:  BitClear(nes_system->GetControllerIO()->GetControllerState()->controller_state_port1_D0, NES_CONTROLLER_DOWN); break;
			case SDLK_LEFT:  BitClear(nes_system->GetControllerIO()->GetControllerState()->controller_state_port1_D0, NES_CONTROLLER_LEFT); break;
			case SDLK_RIGHT: BitClear(nes_system->GetControllerIO()->GetControllerState()->controller_state_port1_D0, NES_CONTROLLER_RIGHT); break;
			case SDLK_z:     BitClear(nes_system->GetControllerIO()->GetControllerState()->controller_state_port1_D0, NES_CONTROLLER_SELECT); break;
			case SDLK_x:     BitClear(nes_system->GetControllerIO()->GetControllerState()->controller_state_port1_D0, NES_CONTROLLER_START); break;
			case SDLK_c:     BitClear(nes_system->GetControllerIO()->GetControllerState()->controller_state_port1_D0, NES_CONTROLLER_B); break;
			case SDLK_v:     BitClear(nes_system->GetControllerIO()->GetControllerState()->controller_state_port1_D0, NES_CONTROLLER_A); break;
			default: break;
		}
	}
}

void Emulator::HandleCommandLine() {
	std::cout << "Working directory: " << stored_argv[0] << '\n';

	for(size_t i = 1; i < stored_argc; i++) {
		std::cout << "Argument " << i << ": " << stored_argv[i] << '\n';
	}
}
