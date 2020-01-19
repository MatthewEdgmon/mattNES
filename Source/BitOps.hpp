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

#ifndef __BITOPS_HPP__
#define __BITOPS_HPP__

#include <cstdint>

/* (uintmax_t) used to be ultra sure it works for all types. */

template<typename T, typename U> inline void BitSet(T &variable, U position) {
	variable |= ((uintmax_t)1 << position);
}

template<typename T, typename U> inline void BitClear(T &variable, U position) {
	variable &= ~((uintmax_t)1 << position);
}

template<typename T, typename U> inline void BitFlip(T &variable, U position) {
	variable ^= ((uintmax_t)1 << position);
}

template<typename T, typename U> inline bool BitCheck(T variable, U position) {
	return (variable >> position) & (uintmax_t)1;
}

#endif __BITOPS_HPP__