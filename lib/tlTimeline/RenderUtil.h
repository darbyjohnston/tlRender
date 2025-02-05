// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTimeline/IRender.h>

namespace tl
{
    namespace timeline
    {
        //! Set and restore the render size.
        class RenderSizeState
        {
        public:
            RenderSizeState(const std::shared_ptr<IRender>&);

            ~RenderSizeState();

        private:
            DTK_PRIVATE();
        };

        //! Set and restore the viewport.
        class ViewportState
        {
        public:
            ViewportState(const std::shared_ptr<IRender>&);

            ~ViewportState();

        private:
            DTK_PRIVATE();
        };

        //! Set and restore whether the clipping rectangle is enabled.
        class ClipRectEnabledState
        {
        public:
            ClipRectEnabledState(const std::shared_ptr<IRender>&);

            ~ClipRectEnabledState();

        private:
            DTK_PRIVATE();
        };

        //! Set and restore the clipping rectangle.
        class ClipRectState
        {
        public:
            ClipRectState(const std::shared_ptr<IRender>&);

            ~ClipRectState();

            const dtk::Box2I& getClipRect() const;

        private:
            DTK_PRIVATE();
        };

        //! Set and restore the transform.
        class TransformState
        {
        public:
            TransformState(const std::shared_ptr<IRender>&);

            ~TransformState();

        private:
            DTK_PRIVATE();
        };

        //! Get a box with the given aspect ratio that fits within
        //! the given box.
        dtk::Box2I getBox(float aspect, const dtk::Box2I&);
    }
}
