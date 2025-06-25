// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlTimeline/RenderUtil.h>

namespace tl
{
    namespace timeline
    {
        feather_tk::Box2I getBox(float aspect, const feather_tk::Box2I& box)
        {
            feather_tk::Box2I out;
            const feather_tk::Size2I boxSize = box.size();
            const float boxAspect = feather_tk::aspectRatio(boxSize);
            if (boxAspect > aspect)
            {
                out = feather_tk::Box2I(
                    box.min.x + boxSize.w / 2.F - (boxSize.h * aspect) / 2.F,
                    box.min.y,
                    boxSize.h * aspect,
                    boxSize.h);
            }
            else
            {
                out = feather_tk::Box2I(
                    box.min.x,
                    box.min.y + boxSize.h / 2.F - (boxSize.w / aspect) / 2.F,
                    boxSize.w,
                    boxSize.w / aspect);
            }
            return out;
        }

    }
}
