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

#include <cmath>
#include <iostream>
#include <stdio.h>

#include <SDL.h>

#include "BitOps.hpp"
#include "NES/NESSystem.hpp"
#include "NES/ControllerIO.hpp"
#include "NES/APU.hpp"
#include "NES/CPU.hpp"
#include "NES/PPU.hpp"
#include "Emulator.hpp"

int TestThread(void* data) {
	return 0;
}

void EmulatorAudioCallback(void* userdata, Uint8* stream, int length) {
	/* Yeehaw */
	const auto emulator_instance = reinterpret_cast<Emulator*>(userdata);
	emulator_instance->AudioCallback(stream, length);
}

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

	std::cout << "Using SDL2 engine." << std::dec << '\n';
	std::cout << "Compiled with SDL2 version: " << unsigned(sdl_compiled_version.major) << "." << unsigned(sdl_compiled_version.minor) << "." << unsigned(sdl_compiled_version.patch) << '\n';
	std::cout << "Linked with SDL2 version:   " << unsigned(sdl_linked_version.major) << "." << unsigned(sdl_linked_version.minor) << "." << unsigned(sdl_linked_version.patch) << '\n';

	SDL_DisableScreenSaver();

	sdl_window = SDL_CreateWindow("mattNES", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, PPU::ScreenWidth, PPU::ScreenHeight, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI);

	sdl_renderer = SDL_CreateRenderer(sdl_window, -1, SDL_RENDERER_ACCELERATED);

	SDL_GetRendererInfo(sdl_renderer, &sdl_renderer_info);

	std::cout << "Renderer Name: " << sdl_renderer_info.name << '\n';
	std::cout << "Renderer Texture Formats: ";

	for(size_t i = 0; i < sdl_renderer_info.num_texture_formats; i++) {
		std::cout << SDL_GetPixelFormatName(sdl_renderer_info.texture_formats[i]) << " ";
	}

	std::cout << '\n';

	sdl_texture = SDL_CreateTexture(sdl_renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, PPU::ScreenWidth, PPU::ScreenHeight);

	/* Open audio. */
	SDL_AudioSpec want, have;

	SDL_zero(want);
	want.freq     = 44100;
	want.format   = AUDIO_S16SYS;
	want.channels = 1;
	want.samples  = 2048;
	want.userdata = this;
	want.callback = &EmulatorAudioCallback;

	sdl_audio_device_id = SDL_OpenAudioDevice(NULL, 0, &want, &have, 0);

	if(sdl_audio_device_id == 0) {
		std::cout << "SDL_OpenAudioDevice() failed! Error: " << SDL_GetError() << '\n';
	}

	/* Begin playing audio. */
	//SDL_PauseAudioDevice(sdl_audio_device_id, 0);

	/* Open log file. */
	log_file.open("LogFile.txt", std::ios::out | std::ios::trunc);

	/* Create emulation thread. */
	sdl_emulation_thread = SDL_CreateThread(TestThread, "mattNES Emulation Thread", nullptr);

	SDL_WaitThread(sdl_emulation_thread, &sdl_emulation_thread_value);

	// SDL Initializtion Finished Here.
	
	/* Test ROMs */
	//file_name = "Test/instr_test-v3/all_instrs.nes";
	//file_name = "Test/instr_test-v5/official_only.nes";
	file_name = "Test/instr_test-v3/rom_singles/01-implied.nes";
	//file_name = "Test/instr_test-v3/rom_singles/02-immediate.nes";
	//file_name = "Test/instr_test-v3/rom_singles/03-zero_page.nes";
	//file_name = "Test/instr_test-v3/rom_singles/04-zp_xy.nes";
	//file_name = "Test/instr_test-v3/rom_singles/05-absolute.nes";
	//file_name = "Test/instr_test-v3/rom_singles/06-abs_xy.nes";
	//file_name = "Test/instr_test-v3/rom_singles/07-ind_x.nes";
	//file_name = "Test/instr_test-v3/rom_singles/08-ind_y.nes";
	//file_name = "Test/instr_test-v3/rom_singles/09-branches.nes";
	//file_name = "Test/instr_test-v3/rom_singles/10-stack.nes";
	//file_name = "Test/instr_test-v3/rom_singles/11-jmp_jsr.nes";
	//file_name = "Test/instr_test-v3/rom_singles/12-rts.nes";
	//file_name = "Test/instr_test-v3/rom_singles/13-rti.nes";
	//file_name = "Test/instr_test-v3/rom_singles/14-brk.nes";
	//file_name = "Test/instr_test-v3/rom_singles/15-special.nes";

	//file_name = "Test/other/nestest.nes";

	//file_name = "Test/instr_misc/instr_misc.nes");

	//file_name = "Test/ppu_vbl_nmi/ppu_vbl_nmi.nes";
	//file_name = "Test/nrom368/test1.nes";
	//file_name = "Test/spritecans-2011/spritecans.nes";

	/* NROM Games */
	//file_name = "ROMs/Baseball.nes";
	//file_name = "ROMs/Donkey Kong.nes";
	//file_name = "ROMs/Mario Bros..nes";
	//file_name = "ROMs/Pinball.nes";

	/* Hard NROM Games */
	//file_name = "ROMs/Super Mario Bros..nes";

	/* To test dummy APU IRQ reads. */
	//ironsword.nes
	//cobra triangles.nes

	// TODO: nestest.nes Nintendulator log expects opcode ISC to be named "ISB", decide which I actually want.
	// TODO: Seperate Interrupt() into two, one for requesting the interrupt, one for actually handling it, and a bool owned by the class for checking whether an interrupt is pending.
	// TODO: Illegal opcodes that combine two instructions need to perform Read and Write the correct number of times, not just at the start of the instruction.

	/* Create emulated system. */
	nes_system = std::make_unique<NESSystem>(NESSystem::CPUEmulationMode::RP2A03, NESSystem::PPUEmulationMode::RP2C02, NESSystem::RegionEmulationMode::NTSC);
	nes_system->Initialize(file_name);

	/* Set program counter to automated mode for nestest.nes */
	if(file_name == "Test/other/nestest.nes") {
		nes_system->GetCPU()->SetProgramCounter(0xC000);
	}

	is_fully_initialized = true;
}

void Emulator::Loop() {

	// TODO: Handle these elsewhere.
	
	display_framerate = false;
	display_video = true;
	emulation_paused = false;
	single_step = false;
	disassemble_cpu = false;
	log_to_file = false;
	is_running = true;

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
					break;
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

			if(disassemble_cpu) {
				std::cout << nes_system->GetCPU()->StepDisassembler();

				if(log_to_file) {
					log_file << nes_system->GetCPU()->StepDisassembler();
				}
			}

			nes_system->Frame();
		}

		if(single_step) {
		
			if(disassemble_cpu) {
				std::cout << nes_system->GetCPU()->StepDisassembler();

				if(log_to_file) {
					log_file << nes_system->GetCPU()->StepDisassembler();
				}
			}

			nes_system->Frame();

			single_step = false;
		}

		/* Pause at end of automated test for nestest.nes */
		if(file_name == "Test/other/nestest.nes" && nes_system->GetCPU()->GetProgramCounter() == 0xC66E) {
			emulation_paused = true;
		}

		/* Render graphics. */
		SDL_RenderClear(sdl_renderer);

		if(display_video) {
			SDL_UpdateTexture(sdl_texture, NULL, nes_system->GetPPU()->GetVideoBuffer(), (nes_system->GetPPU()->ScreenWidth * 4));
		}

		SDL_RenderCopy(sdl_renderer, sdl_texture, NULL, NULL);

		SDL_RenderPresent(sdl_renderer);

		frame_end = SDL_GetPerformanceCounter();
		
		perf_freq = SDL_GetPerformanceFrequency();
		frame_time = (frame_end - frame_start) / static_cast<double>(perf_freq);
		frame_rate = 1 / (frame_time * 1000.0);

		/* Only update the cycle counter every 1000 CPU clocks. */
		if(nes_system->GetCPU()->CycleCount() % 1000 == 0) {
			//sprintf(title_buffer, "%f", (frame_time * 1000.0));
			//sprintf(title_buffer, "%f", frame_rate);
			//sprintf_s(title_buffer, "%lli", nes_system->GetCPU()->CycleCount());
			SDL_SetWindowTitle(sdl_window, title_buffer);
		}

		if(display_framerate) {
			std::cout << "Frame Time: " << frame_time * 1000.0 << "ms" << '\n';
		}

		//SDL_Delay(1);
	}
}

void Emulator::Shutdown() {
	
	/* Signal to others application is going down. */
	is_fully_initialized = false;

	/* Shutdown emulated system. */
	nes_system->Shutdown();

	/* Close log file. */
	log_file.close();

	/* Close audio. */
	SDL_PauseAudioDevice(sdl_audio_device_id, 1);

	SDL_CloseAudioDevice(sdl_audio_device_id);

	/* Cleanup graphics. */
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
			single_step = true;
			break;
		case SDLK_ESCAPE:
			is_running = false;
			break;
		case SDLK_d:
			disassemble_cpu = !disassemble_cpu;
			break;
		case SDLK_e:
			emulation_paused = !emulation_paused;
			break;
		case SDLK_f:
			display_framerate = !display_framerate;
			break;
		case SDLK_v:
			display_video = !display_video;
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

void Emulator::AudioCallback(std::uint8_t* stream, int length) {

	if(!is_fully_initialized) {
		return;
	}

	std::int16_t* buffer = (std::int16_t*) stream;

	/* 2 bytes per sample for AUDIO_S16SYS */
	length = length / 2;

	int sample_number = 0;
	
	auto apu_output = nes_system->GetAPU()->GetOutputState();

	/* Mix in the audio... */
	for(int i = 0; i < length; i++, sample_number++) {
		double my_time = (double) sample_number / (double) 44100;
		buffer[i] = (int16_t)(28000 * sin(2.0f * M_PI * 220.5f * my_time));
		//buffer[i] = apu_output;
	}
}