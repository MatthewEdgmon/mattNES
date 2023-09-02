cmake_minimum_required(VERSION 3.8)
project(mattNES CXX)

# Include custom modules.
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/CMake")

# Find SDL2.
find_package(SDL2 REQUIRED COMPONENTS main)

# Write CMakeConfig.hpp.
message(STATUS "Generating header config file: ${CMAKE_SOURCE_DIR}/Source/CMakeConfig.hpp")
set(TEST_CONFIG_OPTION 0 CACHE BOOL "Testing CMake config option")
configure_file(CMakeConfig.hpp.in "${CMAKE_SOURCE_DIR}/Source/CMakeConfig.hpp")

# Define source files.
set(SOURCE_FILES Source/Main.cpp
                 Source/BitOps.hpp
                 Source/CMakeConfig.hpp
                 Source/Emulator.cpp
                 Source/Emulator.hpp
                 Source/HexOutput.hpp
                 Source/NES/Mappers/Mapper.hpp
                 Source/NES/Mappers/MapperMMC1.cpp
                 Source/NES/Mappers/MapperMMC1.hpp
                 Source/NES/Mappers/MapperMMC5.cpp
                 Source/NES/Mappers/MapperMMC5.hpp
                 Source/NES/Mappers/MapperNROM.cpp
                 Source/NES/Mappers/MapperNROM.hpp
                 Source/NES/APU.cpp
                 Source/NES/APU.hpp
                 Source/NES/Cartridge.cpp
                 Source/NES/Cartridge.hpp
                 Source/NES/ControllerIO.cpp
                 Source/NES/ControllerIO.hpp
                 Source/NES/CPU_Disassemble.cpp
                 Source/NES/CPU_Instructions.cpp
                 Source/NES/CPU.cpp
                 Source/NES/CPU.hpp
                 Source/NES/DynaRecEngine.cpp
                 Source/NES/DynaRecEngine.hpp
                 Source/NES/iNESHeader.hpp
                 Source/NES/NESSystem.cpp
                 Source/NES/NESSystem.hpp
                 Source/NES/PPU_IO.cpp
                 Source/NES/PPU.cpp
                 Source/NES/PPU.hpp
                 Source/NES/UNIFHeader.hpp)

# Define executable.
include_directories(${SDL2_INCLUDE_DIRS} ${SDL2main_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR})
add_executable(mattNES ${SOURCE_FILES})
target_link_libraries(mattNES ${SDL2_LIBS})

# Set up Visual Studio filters.
function(assign_source_group)
    foreach(_source IN ITEMS ${ARGN})
        if (IS_ABSOLUTE "${_source}")
            file(RELATIVE_PATH _source_rel "${CMAKE_CURRENT_SOURCE_DIR}" "${_source}")
        else()
            set(_source_rel "${_source}")
        endif()
        get_filename_component(_source_path "${_source_rel}" PATH)
        string(REPLACE "/" "\\" _source_path_msvc "${_source_path}")
        source_group("${_source_path_msvc}" FILES "${_source}")
    endforeach()
endfunction(assign_source_group)

assign_source_group(${SOURCE_FILES})

# Set Visual Studio working directory to the base source directory.
set_target_properties(mattNES PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")

# Set Visual Studio startup project.
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT mattNES)

# Copy SDL2 DLLs to output folder on Windows.
message(STATUS "Copying SDL2 DLLs to build output directory.")
if(WIN32)
    foreach(DLL ${SDL2_DLLS})
        message(STATUS "${DLL}")
        message(STATUS "${CMAKE_GENERATOR_PLATFORM}")
        add_custom_command(TARGET mattNES POST_BUILD COMMAND
            ${CMAKE_COMMAND} -E copy_if_different ${DLL} $<TARGET_FILE_DIR:mattNES>)
    endforeach()
endif()
