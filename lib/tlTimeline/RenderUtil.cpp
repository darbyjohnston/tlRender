// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlTimeline/RenderUtil.h>

namespace tl
{
    namespace timeline
    {
        ftk::Box2I getBox(float aspect, const ftk::Box2I& box)
        {
            ftk::Box2I out;
            const ftk::Size2I boxSize = box.size();
            const float boxAspect = ftk::aspectRatio(boxSize);
            if (boxAspect > aspect)
            {
                out = ftk::Box2I(
                    box.min.x + boxSize.w / 2.F - (boxSize.h * aspect) / 2.F,
                    box.min.y,
                    boxSize.h * aspect,
                    boxSize.h);
            }
            else
            {
                out = ftk::Box2I(
                    box.min.x,
                    box.min.y + boxSize.h / 2.F - (boxSize.w / aspect) / 2.F,
                    boxSize.w,
                    boxSize.w / aspect);
            }
            return out;
        }

    }
}
