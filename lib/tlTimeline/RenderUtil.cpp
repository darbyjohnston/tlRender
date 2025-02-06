// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlTimeline/RenderUtil.h>

namespace tl
{
    namespace timeline
    {
        dtk::Box2I getBox(float aspect, const dtk::Box2I& box)
        {
            dtk::Box2I out;
            const dtk::Size2I boxSize = box.size();
            const float boxAspect = dtk::aspectRatio(boxSize);
            if (boxAspect > aspect)
            {
                out = dtk::Box2I(
                    box.min.x + boxSize.w / 2.F - (boxSize.h * aspect) / 2.F,
                    box.min.y,
                    boxSize.h * aspect,
                    boxSize.h);
            }
            else
            {
                out = dtk::Box2I(
                    box.min.x,
                    box.min.y + boxSize.h / 2.F - (boxSize.w / aspect) / 2.F,
                    boxSize.w,
                    boxSize.w / aspect);
            }
            return out;
        }

    }
}
