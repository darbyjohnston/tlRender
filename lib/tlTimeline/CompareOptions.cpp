// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlTimeline/CompareOptions.h>

#include <dtk/core/Error.h>
#include <dtk/core/String.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdlib>

namespace tl
{
    namespace timeline
    {
        DTK_ENUM_IMPL(
            CompareMode,
            "A",
            "B",
            "Wipe",
            "Overlay",
            "Difference",
            "Horizontal",
            "Vertical",
            "Tile");

        DTK_ENUM_IMPL(
            CompareTimeMode,
            "Relative",
            "Absolute");

        std::vector<dtk::Box2I> getBoxes(CompareMode mode, const std::vector<dtk::ImageInfo>& infos)
        {
            std::vector<dtk::Box2I> out;
            const size_t count = infos.size();
            switch (mode)
            {
            case CompareMode::Horizontal:
            {
                dtk::ImageInfo info;
                if (count > 0)
                {
                    info = infos[0];
                }
                if (count > 0)
                {
                    out.push_back(dtk::Box2I(
                        0,
                        0,
                        info.size.w * info.pixelAspectRatio,
                        info.size.h));
                }
                if (count > 1)
                {
                    out.push_back(dtk::Box2I(
                        info.size.w * info.pixelAspectRatio,
                        0,
                        info.size.w * info.pixelAspectRatio,
                        info.size.h));
                }
                break;
            }
            case CompareMode::Vertical:
            {
                dtk::ImageInfo info;
                if (count > 0)
                {
                    info = infos[0];
                }
                if (count > 0)
                {
                    out.push_back(dtk::Box2I(
                        0,
                        0,
                        info.size.w * info.pixelAspectRatio,
                        info.size.h));
                }
                if (count > 1)
                {
                    out.push_back(dtk::Box2I(
                        0,
                        info.size.h,
                        info.size.w * info.pixelAspectRatio,
                        info.size.h));
                }
                break;
            }
            case CompareMode::Tile:
                if (count > 0)
                {
                    dtk::Size2I tileSize;
                    float pixelAspectRatio = 1.F;
                    for (const auto& info : infos)
                    {
                        if (dtk::area(info.size) > dtk::area(tileSize))
                        {
                            tileSize = info.size;
                        }
                        pixelAspectRatio = std::max(pixelAspectRatio, info.pixelAspectRatio);
                    }

                    int columns = 0;
                    int rows = 0;
                    switch (count)
                    {
                    case 1: columns = 1; rows = 1; break;
                    case 2: columns = 1; rows = 2; break;
                    default:
                    {
                        const float sqrt = std::sqrt(count);
                        columns = std::ceil(sqrt);
                        const std::div_t d = std::div(count, columns);
                        rows = d.quot + (d.rem > 0 ? 1 : 0);
                        break;
                    }
                    }

                    int i = 0;
                    for (int r = 0, y = 0; r < rows; ++r)
                    {
                        for (int c = 0, x = 0; c < columns; ++c, ++i)
                        {
                            if (i < count)
                            {
                                const auto& info = infos[i];
                                const dtk::Box2I box(
                                    x,
                                    y,
                                    tileSize.w * pixelAspectRatio,
                                    tileSize.h);
                                out.push_back(box);
                            }
                            x += tileSize.w * pixelAspectRatio;
                        }
                        y += tileSize.h;
                    }
                }
                break;
            default:
                for (size_t i = 0; i < std::min(count, static_cast<size_t>(2)); ++i)
                {
                    out.push_back(dtk::Box2I(
                        0,
                        0,
                        infos[0].size.w * infos[0].pixelAspectRatio,
                        infos[0].size.h));
                }
                break;
            }
            return out;
        }

        std::vector<dtk::Box2I> getBoxes(CompareMode mode, const std::vector<VideoData>& videoData)
        {
            std::vector<dtk::ImageInfo> infos;
            for (const auto& i : videoData)
            {
                dtk::ImageInfo info;
                if (!i.layers.empty() && i.layers.front().image)
                {
                    info = i.layers.front().image->getInfo();
                }
                infos.push_back(info);
            }
            return getBoxes(mode, infos);
        }

        dtk::Size2I getRenderSize(CompareMode mode, const std::vector<dtk::ImageInfo>& infos)
        {
            dtk::Size2I out;
            dtk::Box2I box;
            const auto boxes = getBoxes(mode, infos);
            if (!boxes.empty())
            {
                box = boxes[0];
                for (size_t i = 1; i < boxes.size(); ++i)
                {
                    box = dtk::expand(box, boxes[i]);
                }
                out.w = box.w();
                out.h = box.h();
            }
            return out;
        }

        dtk::Size2I getRenderSize(CompareMode mode, const std::vector<VideoData>& videoData)
        {
            std::vector<dtk::ImageInfo> infos;
            for (const auto& i : videoData)
            {
                dtk::ImageInfo info;
                if (!i.layers.empty() && i.layers.front().image)
                {
                    info = i.layers.front().image->getInfo();
                }
                infos.push_back(info);
            }
            return getRenderSize(mode, infos);
        }

        OTIO_NS::RationalTime getCompareTime(
            const OTIO_NS::RationalTime& sourceTime,
            const OTIO_NS::TimeRange& sourceTimeRange,
            const OTIO_NS::TimeRange& compareTimeRange,
            CompareTimeMode mode)
        {
            OTIO_NS::RationalTime out;
            switch (mode)
            {
            case CompareTimeMode::Relative:
            {
                const OTIO_NS::RationalTime relativeTime =
                    sourceTime - sourceTimeRange.start_time();
                const OTIO_NS::RationalTime relativeTimeRescaled = relativeTime.
                    rescaled_to(compareTimeRange.duration().rate()).
                    floor();
                out = compareTimeRange.start_time() + relativeTimeRescaled;
                break;
            }
            case CompareTimeMode::Absolute:
                out = sourceTime.
                    rescaled_to(compareTimeRange.duration().rate()).
                    floor();
                break;
            default: break;
            }
            return out;
        }
    }
}
