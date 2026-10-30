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

#include "FieldTimeline7.hpp"
#include <Core/Gen7/Spot7.hpp>

FieldTimeline7::FieldTimeline7(u32 seed, u32 advance) : sfmt(seed, advance), advance(advance)
{
}

u64 FieldTimeline7::next()
{
    advance++;
    return sfmt.next();
}

void FieldTimeline7::blink(Model &model)
{
    // Handoff 3.2: state 0 waits, then rand(128) == 0 starts a blink; states 1-4 are silent; state 5 draws rand(3)
    // (0 -> wait 60 and a double blink, else wait 30), unless the double-blink flag is set
    if (model.state == 0)
    {
        if (model.doubleBlink)
        {
            model.state = 1;
            return;
        }
        if (model.wait > 0)
        {
            model.wait--;
            if (model.wait > 0)
            {
                return;
            }
        }
        if (next() % 128 == 0)
        {
            model.state = 1;
        }
    }
    else if (model.state < 5)
    {
        model.state++;
    }
    else
    {
        if (model.doubleBlink)
        {
            model.wait = 30;
            model.doubleBlink = false;
        }
        else if (next() % 3 == 0)
        {
            model.wait = 60;
            model.doubleBlink = true;
        }
        else
        {
            model.wait = 30;
        }
        model.state = 0;
    }
}

std::vector<FieldTimeline7::Model> FieldTimeline7::rebuild(u8 count)
{
    std::vector<Model> models(count);
    for (Model &model : models)
    {
        blink(model);
    }
    return models;
}

void FieldTimeline7::frame(std::vector<Model> &models, u32 persist, bool rain)
{
    for (size_t i = 0; i < models.size() && i < 32; i++)
    {
        if ((persist >> i) & 1)
        {
            blink(models[i]);
        }
    }
    if (rain)
    {
        next();
        next();
    }
}

std::vector<u32> FieldTimeline7::continueAdvances(u32 seed, u32 advance, const Spot7 &spot, u32 frames, u32 menuFrame, bool boy, bool rain)
{
    FieldTimeline7 timeline(seed, advance);
    std::vector<u32> advances;
    advances.reserve(frames + 1);

    // Load-time draws (fishing-spot lottery and the like) come before field frame 1
    for (u8 i = 0; i < spot.continueLoadDraws; i++)
    {
        timeline.next();
    }
    advances.emplace_back(timeline.getAdvance());

    bool single = spot.flags & Spot7::FidgetSingle;
    u32 nextFidget = spot.fidget;
    std::vector<Model> models;
    for (u32 f = 1; f <= frames; f++)
    {
        // (b) the model eye updates: all L models on frame 1, the persistent ones after
        if (f == 1)
        {
            models = timeline.rebuild(spot.continueModels);
            // (d) rain pair from field frame 1, every second frame
            if (rain)
            {
                timeline.next();
                timeline.next();
            }
        }
        else
        {
            timeline.frame(models, spot.continuePersist, rain && ((f - 1) % 2 == 0));
        }

        // (e) the fidget: rand(90) (+ rand(2) for the animation), repeating after rand(90) + 407/412 (boy) or 402/407 (girl) frames;
        // hot/cold zones draw one rand(90) and repeat after rand(90) + 402. The X menu stops the timer.
        if (nextFidget != 0 && f == nextFidget && (menuFrame == 0 || f <= menuFrame))
        {
            u32 r = static_cast<u32>(timeline.next() % 90);
            if (single)
            {
                nextFidget = f + r + 402;
            }
            else
            {
                u32 animation = static_cast<u32>(timeline.next() % 2);
                nextFidget = f + r + (boy ? (animation ? 412 : 407) : (animation ? 407 : 402));
            }
        }
        advances.emplace_back(timeline.getAdvance());
    }
    return advances;
}
