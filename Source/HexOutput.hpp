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

#ifndef __HEX_OUTPUT_HPP__
#define __HEX_OUTPUT_HPP__

#include <iomanip>

#define HEX(x) "0x" << std::setfill('0') << std::setw(4) << std::hex << std::uppercase << unsigned(x) << std::dec << std::nouppercase

#define HEX4(x) "0x" << std::setfill('0') << std::setw(4) << std::hex << std::uppercase << unsigned(static_cast<uint16_t>(x)) << std::dec << std::nouppercase
#define HEX2(x) "0x" << std::setfill('0') << std::setw(2) << std::hex << std::uppercase << unsigned(static_cast<uint8_t>(x)) << std::dec << std::nouppercase

/* These are used for disassembly formatting. */

#define HEX4X(x) std::setfill('0') << std::setw(4) << std::hex << std::uppercase << unsigned(static_cast<uint16_t>(x)) << std::dec << std::nouppercase
#define HEX2X(x) std::setfill('0') << std::setw(2) << std::hex << std::uppercase << unsigned(static_cast<uint8_t>(x)) << std::dec << std::nouppercase

#endif /* __HEX_OUTPUT_HPP__ */
