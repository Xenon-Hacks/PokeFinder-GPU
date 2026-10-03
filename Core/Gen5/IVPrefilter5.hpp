/*
 * This file is part of PokéFinder
 * Copyright (C) 2017-2026 by Admiral_Fish, bumba, and EzPzStreamz
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef IVPREFILTER5_HPP
#define IVPREFILTER5_HPP

#include <Core/Global.hpp>
#include <array>

/**
 * @brief Describes how a Gen 5 generator reads IVs from MT, so a searcher can filter IVs ahead of the generator
 * (for example on the GPU) and hand it only the IV advances that can pass
 */
struct IVPrefilter5
{
    std::array<u8, 6> ivMin; ///< Per stat minimum IVs, PokeFinder stat order
    std::array<u8, 6> ivMax; ///< Per stat maximum IVs, PokeFinder stat order
    u8 offset; ///< MT outputs skipped before the first IV at IV advance 0
    bool roamer; ///< Roamer IV order (HP, Atk, Def, SpD, Spe, SpA)
    bool supported; ///< Whether the generator only keeps IV advances that pass the IV filter
};

#endif // IVPREFILTER5_HPP
