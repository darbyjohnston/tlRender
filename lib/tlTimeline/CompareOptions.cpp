// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlTimeline/CompareOptions.h>

#include <feather-tk/core/Error.h>
#include <feather-tk/core/String.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdlib>
#include <sstream>

namespace tl
{
    namespace timeline
    {
        FTK_ENUM_IMPL(
            Compare,
            "A",
            "B",
            "Wipe",
            "Overlay",
            "Difference",
            "Horizontal",
            "Vertical",
            "Tile");

        FTK_ENUM_IMPL(
            CompareTime,
            "Relative",
            "Absolute");

        bool CompareOptions::operator == (const CompareOptions& other) const
        {
            return
                compare == other.compare &&
                wipeCenter == other.wipeCenter &&
                wipeRotation == other.wipeRotation &&
                overlay == other.overlay;
        }

        bool CompareOptions::operator != (const CompareOptions& other) const
        {
            return !(*this == other);
        }

        std::vector<ftk::Box2I> getBoxes(Compare compare, const std::vector<ftk::ImageInfo>& infos)
        {
            std::vector<ftk::Box2I> out;
            const size_t count = infos.size();
            switch (compare)
            {
            case Compare::Horizontal:
            {
                ftk::ImageInfo info;
                if (count > 0)
                {
                    info = infos[0];
                }
                if (count > 0)
                {
                    out.push_back(ftk::Box2I(
                        0,
                        0,
                        info.size.w * info.pixelAspectRatio,
                        info.size.h));
                }
                if (count > 1)
                {
                    out.push_back(ftk::Box2I(
                        info.size.w * info.pixelAspectRatio,
                        0,
                        info.size.w * info.pixelAspectRatio,
                        info.size.h));
                }
                break;
            }
            case Compare::Vertical:
            {
                ftk::ImageInfo info;
                if (count > 0)
                {
                    info = infos[0];
                }
                if (count > 0)
                {
                    out.push_back(ftk::Box2I(
                        0,
                        0,
                        info.size.w * info.pixelAspectRatio,
                        info.size.h));
                }
                if (count > 1)
                {
                    out.push_back(ftk::Box2I(
                        0,
                        info.size.h,
                        info.size.w * info.pixelAspectRatio,
                        info.size.h));
                }
                break;
            }
            case Compare::Tile:
                if (count > 0)
                {
                    ftk::Size2I tileSize;
                    float pixelAspectRatio = 1.F;
                    for (const auto& info : infos)
                    {
                        if (ftk::area(info.size) > ftk::area(tileSize))
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
                                const ftk::Box2I box(
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
                    out.push_back(ftk::Box2I(
                        0,
                        0,
                        infos[0].size.w * infos[0].pixelAspectRatio,
                        infos[0].size.h));
                }
                break;
            }
            return out;
        }

        std::vector<ftk::Box2I> getBoxes(Compare compare, const std::vector<VideoData>& videoData)
        {
            std::vector<ftk::ImageInfo> infos;
            for (const auto& i : videoData)
            {
                ftk::ImageInfo info;
                for (const auto& layer : i.layers)
                {
                    if (layer.image)
                    {
                        info = layer.image->getInfo();
                        break;
                    }
                    else if (layer.imageB)
                    {
                        info = layer.imageB->getInfo();
                        break;
                    }
                }
                infos.push_back(info);
            }
            return getBoxes(compare, infos);
        }

        ftk::Size2I getRenderSize(Compare compare, const std::vector<ftk::ImageInfo>& infos)
        {
            ftk::Size2I out;
            ftk::Box2I box;
            const auto boxes = getBoxes(compare, infos);
            if (!boxes.empty())
            {
                box = boxes[0];
                for (size_t i = 1; i < boxes.size(); ++i)
                {
                    box = ftk::expand(box, boxes[i]);
                }
                out.w = box.w();
                out.h = box.h();
            }
            return out;
        }

        ftk::Size2I getRenderSize(Compare compare, const std::vector<VideoData>& videoData)
        {
            std::vector<ftk::ImageInfo> infos;
            for (const auto& i : videoData)
            {
                ftk::ImageInfo info;
                for (const auto& layer : i.layers)
                {
                    if (layer.image)
                    {
                        info = layer.image->getInfo();
                        break;
                    }
                    else if (layer.imageB)
                    {
                        info = layer.imageB->getInfo();
                        break;
                    }
                }
                infos.push_back(info);
            }
            return getRenderSize(compare, infos);
        }

        OTIO_NS::RationalTime getCompareTime(
            const OTIO_NS::RationalTime& sourceTime,
            const OTIO_NS::TimeRange& sourceTimeRange,
            const OTIO_NS::TimeRange& compareTimeRange,
            CompareTime compare)
        {
            OTIO_NS::RationalTime out;
            switch (compare)
            {
            case CompareTime::Relative:
            {
                const OTIO_NS::RationalTime relativeTime =
                    sourceTime - sourceTimeRange.start_time();
                const OTIO_NS::RationalTime relativeTimeRescaled = relativeTime.
                    rescaled_to(compareTimeRange.duration().rate()).
                    floor();
                out = compareTimeRange.start_time() + relativeTimeRescaled;
                break;
            }
            case CompareTime::Absolute:
                out = sourceTime.
                    rescaled_to(compareTimeRange.duration().rate()).
                    floor();
                break;
            default: break;
            }
            return out;
        }

        void to_json(nlohmann::json& json, const CompareOptions& in)
        {
            json["Compare"] = to_string(in.compare);
            json["WipeCenter"] = in.wipeCenter;
            json["WipeRotation"] = in.wipeRotation;
            json["Overlay"] = in.overlay;
        }

        void from_json(const nlohmann::json& json, CompareOptions& out)
        {
            from_string(json.at("Compare").get<std::string>(), out.compare);
            json.at("WipeCenter").get_to(out.wipeCenter);
            json.at("WipeRotation").get_to(out.wipeRotation);
            json.at("Overlay").get_to(out.overlay);
        }
    }
}
