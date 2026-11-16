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

#include "SharedGenerator8a.hpp"

bool SharedFilter8a::compare(const State8a &state) const
{
    for (int i = 0; i < 6; i++)
    {
        if (state.ivs[i] < ivMin[i] || state.ivs[i] > ivMax[i])
        {
            return false;
        }
    }
    if (!natures[state.nature] || (ability >= 0 && state.ability != ability) || (gender >= 0 && state.gender != gender)
        || (shinyOnly && !state.shiny))
    {
        return false;
    }
    return state.height >= heightMin && state.height <= heightMax && state.weight >= weightMin && state.weight <= weightMax;
}

SharedGenerator8a::SharedGenerator8a(u64 seed, u32 initialAdvance, u32 maxAdvances, const std::vector<Spec8a> &specs,
                                     const SharedFilter8a &filter) :
    specs(specs), filter(filter), seed(seed), initialAdvance(initialAdvance), maxAdvances(maxAdvances)
{
}

std::vector<SharedState8a> SharedGenerator8a::generate() const
{
    std::vector<SharedState8a> states;
    Xoro8a rng(seed);
    rng.advance(initialAdvance);
    rng.draws = 0;

    for (u32 cnt = 0; cnt < maxAdvances; cnt++, rng.next())
    {
        Xoro8a gen(rng.s0, rng.s1);
        u32 advance = initialAdvance + cnt;
        for (u32 i = 0; i < specs.size(); i++)
        {
            State8a state = Generator8a::generate(gen, specs[i], false);
            if (filter.compare(state))
            {
                states.push_back({ advance, i, advance + gen.draws, state });
            }
        }
    }
    return states;
}
