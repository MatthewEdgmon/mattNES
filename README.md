# mattNES

*"An unoriginal name for an unoriginal project!"*

**Please note: This project is primarily a personal learning project for now. Contributions are accepted and welcome, but do not expect accuracy or reliability at this point.**

mattNES is an (unfinished) emulator project that targets any system that has a combination of a Ricoh 2A03 CPU and a Ricoh 2C02 PPU (including revisions of those or clone chips that mimic their behavior). This includes, but is not limited to, the systems below.

1. Nintendo Entertainment System (NES)
2. Nintendo Family Computer (Famicom)
3. Nintendo VS. System
4. Nintendo PlayChoice
5. Steepler Dendy

## Features
1. Most 6502 opcodes written.

## Planned Features
1. Cycle accurate emulation of both CPU and PPU (including illegal opcodes).
2. Automated testing.
3. Rewinding and save states.
4. Support for all regions.
5. Support for all controllers and peripherals.

## Mappers Supported
NROM (mapper 0), MMC1 (mapper 1) is being worked on.

## Build Instructions
The project uses CMake for cross platform building. The only current external dependency is [SDL2](https://libsdl.org/). If installed from source or by package manager, the library should be detected automatically in Linux (**building and running has not been tested on Linux/macOS**). Windows users will have to use Python3 and run `python GetSDL2Win32.py` in a terminal to setup the build environment. If you run the CMake file through Visual Studio directly, it will not set the debugger working directory correctly. Once the emulator is built, the ROM to be run is hardcoded in `Emulator.cpp`. The next version will support dragging and dropping files.

## Testing
Once built, the emulator can be tested with ROMs from [Christopher Pow's NES Test ROMs repository](https://github.com/christopherpow/nes-test-roms). That repository is added as a submodule to this one for your convenience. A script to automate testing of the emulator would be a welcome addition.

## References
Building this project would've been impossible without these resources below.

[NesDev Wiki](http://wiki.nesdev.com/w/index.php/Nesdev_Wiki)

[Andrew Jacobs](http://obelisk.me.uk/6502/)

[javidx9's NES Videos](https://www.youtube.com/channel/UC-yuWVUplUJZvieEligKBkA)

The CMake structure for this project using SDL2 was copied from [Jeff Preshing's CMakeDemo](https://github.com/preshing/CMakeDemo) project.

## Licensing
All mattNES original code is licensed under the GNU GPL v3. See the file [LICENSE.md](../master/LICENSE.md) for more licensing information.
