// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlTimeline/RenderUtil.h>

namespace tl
{
    namespace timeline
    {
        struct RenderSizeState::Private
        {
            std::shared_ptr<IRender> render;
            dtk::Size2I size;
        };

        RenderSizeState::RenderSizeState(const std::shared_ptr<IRender>& render) :
            _p(new Private)
        {
            TLRENDER_P();
            p.render = render;
            p.size = render->getRenderSize();
        }

        RenderSizeState::~RenderSizeState()
        {
            TLRENDER_P();
            p.render->setRenderSize(p.size);
        }

        struct ViewportState::Private
        {
            std::shared_ptr<IRender> render;
            dtk::Box2I viewport;
        };

        ViewportState::ViewportState(const std::shared_ptr<IRender>& render) :
            _p(new Private)
        {
            TLRENDER_P();
            p.render = render;
            p.viewport = render->getViewport();
        }

        ViewportState::~ViewportState()
        {
            TLRENDER_P();
            p.render->setViewport(p.viewport);
        }

        struct ClipRectEnabledState::Private
        {
            std::shared_ptr<IRender> render;
            bool clipRectEnabled = false;
        };

        ClipRectEnabledState::ClipRectEnabledState(const std::shared_ptr<IRender>& render) :
            _p(new Private)
        {
            TLRENDER_P();
            p.render = render;
            p.clipRectEnabled = render->getClipRectEnabled();
        }

        ClipRectEnabledState::~ClipRectEnabledState()
        {
            TLRENDER_P();
            p.render->setClipRectEnabled(p.clipRectEnabled);
        }

        struct ClipRectState::Private
        {
            std::shared_ptr<IRender> render;
            dtk::Box2I clipRect;
        };

        ClipRectState::ClipRectState(const std::shared_ptr<IRender>& render) :
            _p(new Private)
        {
            TLRENDER_P();
            p.render = render;
            p.clipRect = render->getClipRect();
        }

        ClipRectState::~ClipRectState()
        {
            TLRENDER_P();
            p.render->setClipRect(p.clipRect);
        }

        const dtk::Box2I& ClipRectState::getClipRect() const
        {
            return _p->clipRect;
        }

        struct TransformState::Private
        {
            std::shared_ptr<IRender> render;
            dtk::M44F transform;
        };

        TransformState::TransformState(const std::shared_ptr<IRender>& render) :
            _p(new Private)
        {
            TLRENDER_P();
            p.render = render;
            p.transform = render->getTransform();
        }

        TransformState::~TransformState()
        {
            TLRENDER_P();
            p.render->setTransform(p.transform);
        }
        
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
