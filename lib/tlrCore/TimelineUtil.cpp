// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlrCore/TimelineUtil.h>

namespace tlr
{
    namespace timeline
    {
        std::vector<otime::TimeRange> toRanges(std::vector<otime::RationalTime> frames)
        {
            std::vector<otime::TimeRange> out;
            if (!frames.empty())
            {
                std::sort(frames.begin(), frames.end());
                auto i = frames.begin();
                auto j = i;
                do
                {
                    auto k = j + 1;
                    if (k != frames.end() && (*k - *j).value() > 1)
                    {
                        out.push_back(otime::TimeRange::range_from_start_end_time_inclusive(*i, *j));
                        i = k;
                        j = k;
                    }
                    else if (k == frames.end())
                    {
                        out.push_back(otime::TimeRange::range_from_start_end_time_inclusive(*i, *j));
                        i = k;
                        j = k;
                    }
                    else
                    {
                        ++j;
                    }
                } while (j != frames.end());
            }
            return out;
        }

        const otio::Composable* getRoot(const otio::Composable* composable)
        {
            const otio::Composable* out = composable;
            for (; out->parent(); out = out->parent())
                ;
            return out;
        }

        otio::optional<otime::RationalTime> getDuration(const otio::Timeline* timeline, const std::string& kind)
        {
            otio::optional<otime::RationalTime> out;
            otio::ErrorStatus errorStatus;
            for (auto track : timeline->children_if<otio::Track>(&errorStatus))
            {
                if (kind == track->kind())
                {
                    const otime::RationalTime duration = track->duration(&errorStatus);
                    if (out.has_value())
                    {
                        out = std::max(out.value(), duration);
                    }
                    else
                    {
                        out = duration;
                    }
                }
            }
            return out;
        }
    }
}