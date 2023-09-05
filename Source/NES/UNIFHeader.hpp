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

#ifndef __UNIF_HEADER_HPP__
#define __UNIF_HEADER_HPP__

#include <cstdint>

typedef struct unif_header {
	uint32_t signature;
	uint32_t version;
	uint32_t padding1;
	uint32_t padding2;
	uint32_t padding3;
	uint32_t padding4;
	uint32_t padding5;
	uint32_t padding6;
} unif_header_t;

#endif /* __UNIF_HEADER_HPP__ */