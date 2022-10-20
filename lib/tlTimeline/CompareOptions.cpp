// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlTimeline/CompareOptions.h>

#include <tlCore/Error.h>
#include <tlCore/String.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdlib>

namespace tl
{
    namespace timeline
    {
        TLRENDER_ENUM_IMPL(
            CompareMode,
            "A",
            "B",
            "Wipe",
            "Overlay",
            "Difference",
            "Horizontal",
            "Vertical",
            "Tile");
        TLRENDER_ENUM_SERIALIZE_IMPL(CompareMode);

        std::vector<math::BBox2i> tiles(CompareMode mode, const std::vector<imaging::Size>& sizes)
        {
            std::vector<math::BBox2i> out;
            const size_t count = sizes.size();
            switch (mode)
            {
            case CompareMode::Horizontal:
            {
                imaging::Size size;
                if (count > 0)
                {
                    size = sizes[0];
                }
                if (count > 1)
                {
                    size = std::max(size, sizes[1]);
                }
                if (count > 0)
                {
                    out.push_back(math::BBox2i(
                        0,
                        0,
                        size.w * size.pixelAspectRatio,
                        size.h));
                }
                if (count > 1)
                {
                    out.push_back(math::BBox2i(
                        size.w * size.pixelAspectRatio,
                        0,
                        size.w * size.pixelAspectRatio,
                        size.h));
                }
                break;
            }
            case CompareMode::Vertical:
            {
                imaging::Size size;
                if (count > 0)
                {
                    size = sizes[0];
                }
                if (count > 1)
                {
                    size = std::max(size, sizes[1]);
                }
                if (count > 0)
                {
                    out.push_back(math::BBox2i(
                        0,
                        0,
                        size.w * size.pixelAspectRatio,
                        size.h));
                }
                if (count > 1)
                {
                    out.push_back(math::BBox2i(
                        0,
                        size.h,
                        size.w * size.pixelAspectRatio,
                        size.h));
                }
                break;
            }
            case CompareMode::Tile:
                if (count > 0)
                {
                    imaging::Size tileSize;
                    for (const auto& i : sizes)
                    {
                        tileSize = std::max(tileSize, i);
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
                                const auto& s = sizes[i];
                                const math::BBox2i bbox(
                                    x,
                                    y,
                                    tileSize.w * tileSize.pixelAspectRatio,
                                    tileSize.h);
                                out.push_back(bbox);
                            }
                            x += tileSize.w * tileSize.pixelAspectRatio;
                        }
                        y += tileSize.h;
                    }
                }
                break;
            default:
                if (count > 0)
                {
                    out.push_back(math::BBox2i(
                        0,
                        0,
                        sizes[0].w * sizes[0].pixelAspectRatio,
                        sizes[0].h));
                }
                break;
            }
            return out;
        }

        imaging::Size getRenderSize(CompareMode mode, const std::vector<imaging::Size>& sizes)
        {
            imaging::Size out;
            math::BBox2i bbox;
            const auto tiles = timeline::tiles(mode, sizes);
            if (!tiles.empty())
            {
                bbox = tiles[0];
                for (size_t i = 1; i < tiles.size(); ++i)
                {
                    bbox.expand(tiles[i]);
                }
                out.w = bbox.w();
                out.h = bbox.h();
            }
            return out;
        }
    }
}
