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

#ifndef MULTIPROFILESEARCHER5_HPP
#define MULTIPROFILESEARCHER5_HPP

#include <Core/Gen5/Profile5.hpp>
#include <Core/Gen5/Searchers/IVSearcher5GPU.hpp>
#include <Core/Gen5/Searchers/SearcherBase5.hpp>
#include <Core/Gen5/States/SearcherState5.hpp>
#include <Core/Util/DateTime.hpp>
#include <algorithm>
#include <functional>
#include <string>
#include <vector>

/**
 * @brief Runs the same search for several profiles one after another, so a user with several games, consoles or
 * TID/SID combinations gets every hit from one search. Each result is tagged with the index of its profile.
 *
 * @tparam Generator State generator class to use
 * @tparam State State class to use
 */
template <class Generator, class State>
class MultiProfileSearcher5
{
public:
    using Searcher = SearcherBase5<Generator, State>;
    using GPUSearcher = IVSearcher5GPU<Generator, State>;

    /**
     * @brief Searcher made for one profile
     */
    struct Created
    {
        Searcher *searcher = nullptr; ///< CPU searcher, used when \ref gpu is missing or not ready
        GPUSearcher *gpu = nullptr; ///< GPU searcher, if one was attempted
    };

    /**
     * @brief Makes the searcher for a profile, given the profile and its index. Ownership of both searchers passes to the
     * caller.
     */
    using Factory = std::function<Created(const Profile5 &profile, size_t index)>;

    /**
     * @brief Construct a new MultiProfileSearcher5 object
     *
     * @param profiles Profiles to search, in order
     * @param factory Makes the searcher for each profile
     */
    MultiProfileSearcher5(const std::vector<Profile5> &profiles, const Factory &factory) : profiles(profiles), factory(factory)
    {
    }

    ~MultiProfileSearcher5()
    {
        delete searcher;
    }

    MultiProfileSearcher5(const MultiProfileSearcher5 &) = delete;
    MultiProfileSearcher5 &operator=(const MultiProfileSearcher5 &) = delete;

    /**
     * @brief Cancels the running search and skips the profiles not searched yet
     */
    void cancelSearch()
    {
        cancelled = true;
        if (searcher)
        {
            searcher->cancelSearch();
        }
    }

    /**
     * @brief Returns the errors from the GPU, one entry per profile that had any
     *
     * @return Error messages
     */
    std::vector<std::string> getErrors() const
    {
        return errors;
    }

    /**
     * @brief Returns overall progress from 0 to 100
     *
     * @return Progress
     */
    int getProgress() const
    {
        int current = searcher ? searcher->getProgress() : 0;
        return static_cast<int>((current + 100 * static_cast<int>(std::min(next, profiles.size()) - (searcher ? 1 : 0))) / profiles.size());
    }

    /**
     * @brief Returns the results found since the last call, tagged with their profile index
     *
     * @return Results
     */
    std::vector<SearcherState5<State>> getResults()
    {
        std::vector<SearcherState5<State>> results = std::move(pending);
        pending.clear();
        if (searcher)
        {
            append(results, searcher->getResults());
        }
        return results;
    }

    /**
     * @brief Returns the GPU fallback warnings collected since the last call, one entry per profile that fell back to the CPU
     *
     * @return Warning messages
     */
    std::vector<std::string> getWarnings()
    {
        std::vector<std::string> list = std::move(warnings);
        warnings.clear();
        return list;
    }

    /**
     * @brief Checks whether any profile is still being searched. Moves on to the next profile once the current one finishes.
     *
     * @return true Search is still running
     * @return false All profiles finished or the search was cancelled
     */
    bool isSearching()
    {
        if (searcher && searcher->isSearching())
        {
            return true;
        }
        finishCurrent();
        return startNext();
    }

    /**
     * @brief Starts the search
     *
     * @param threads Numbers of threads to search with
     * @param start Start date
     * @param end End date
     *
     * @return true A search was started
     * @return false There was nothing to search
     */
    bool startSearch(int threads, const Date &start, const Date &end)
    {
        this->threads = threads;
        this->start = start;
        this->end = end;
        return startNext();
    }

private:
    std::vector<Profile5> profiles;
    std::vector<SearcherState5<State>> pending;
    std::vector<std::string> errors;
    std::vector<std::string> warnings;
    Factory factory;
    Date start;
    Date end;
    Searcher *searcher = nullptr;
    GPUSearcher *gpu = nullptr;
    size_t next = 0;
    int threads = 1;
    bool cancelled = false;

    /**
     * @brief Appends \p results to \p out, tagged with the profile being searched
     *
     * @param out Result list
     * @param results Results of the current searcher
     */
    void append(std::vector<SearcherState5<State>> &out, std::vector<SearcherState5<State>> &&results) const
    {
        u8 profile = static_cast<u8>(next - 1);
        for (auto &result : results)
        {
            result.setProfile(profile);
        }
        if (out.empty())
        {
            out = std::move(results);
        }
        else
        {
            out.insert(out.end(), results.begin(), results.end());
        }
    }

    /**
     * @brief Keeps the results and GPU error of the finished searcher and frees it
     */
    void finishCurrent()
    {
        if (!searcher)
        {
            return;
        }

        append(pending, searcher->getResults());
        if (gpu)
        {
            auto error = gpu->getError();
            if (!error.empty())
            {
                errors.emplace_back(profileLabel() + error);
            }
        }
        delete searcher;
        searcher = nullptr;
        gpu = nullptr;
    }

    /**
     * @brief Returns the "<profile name>: " prefix for messages when several profiles are searched
     *
     * @return Prefix
     */
    std::string profileLabel() const
    {
        return profiles.size() > 1 ? profiles[next - 1].getName() + ": " : std::string();
    }

    /**
     * @brief Starts searching the next profile
     *
     * @return true A profile is being searched
     * @return false No profiles are left or the search was cancelled
     */
    bool startNext()
    {
        if (cancelled || next >= profiles.size())
        {
            return false;
        }

        const Profile5 &profile = profiles[next];
        Created created = factory(profile, next++);
        if (created.gpu && !created.gpu->isReady())
        {
            warnings.emplace_back(profileLabel() + created.gpu->getError());
            delete created.gpu;
            created.gpu = nullptr;
        }
        else if (created.gpu)
        {
            delete created.searcher;
            created.searcher = created.gpu;
        }

        searcher = created.searcher;
        gpu = created.gpu;
        searcher->setMaxProgress(searcher->getMaxProgress(start, end));
        searcher->startSearch(threads, start, end);
        return true;
    }
};

#endif // MULTIPROFILESEARCHER5_HPP
