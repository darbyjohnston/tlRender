// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlUI/Screenshot.h>

#include <tlGL/OffscreenBuffer.h>
#include <tlGL/Util.h>

#include <tlTimeline/RenderUtil.h>

#include <tlGlad/gl.h>

namespace tl
{
    namespace ui
    {
        namespace
        {
            void drawEvent(
                const std::shared_ptr<IWidget>& widget,
                const math::BBox2i& drawRect,
                const DrawEvent& event)
            {
                if (!widget->isClipped() && widget->getGeometry().isValid())
                {
                    event.render->setClipRect(drawRect);
                    widget->drawEvent(drawRect, event);
                    const math::BBox2i childrenClipRect =
                        widget->getChildrenClipRect().intersect(drawRect);
                    event.render->setClipRect(childrenClipRect);
                    for (const auto& child : widget->getChildren())
                    {
                        const math::BBox2i& childGeometry = child->getGeometry();
                        if (childGeometry.intersects(childrenClipRect))
                        {
                            drawEvent(
                                child,
                                childGeometry.intersect(childrenClipRect),
                                event);
                        }
                    }
                    event.render->setClipRect(drawRect);
                    widget->drawOverlayEvent(drawRect, event);
                }
            }
        }

        std::shared_ptr<imaging::Image> screenshot(
            const std::shared_ptr<IWidget>& widget,
            const imaging::Size& displaySize,
            const std::shared_ptr<Style>& style,
            const std::shared_ptr<IconLibrary>& iconLibrary,
            const std::shared_ptr<timeline::IRender>& render,
            const std::shared_ptr<imaging::FontSystem>& fontSystem,
            float displayScale)
        {
            const math::BBox2i& geometry = widget->getGeometry();
            const imaging::Size size(geometry.w(), geometry.h());
            const imaging::Info info(size, imaging::PixelType::RGBA_U8);
            auto out = imaging::Image::create(info);

            gl::OffscreenBufferOptions options;
            options.colorType = imaging::PixelType::RGBA_F32;
            auto buffer = gl::OffscreenBuffer::create(displaySize, options);

            gl::OffscreenBufferBinding binding(buffer);
            DrawEvent event(
                style,
                iconLibrary,
                render,
                fontSystem,
                displayScale);
            render->begin(displaySize);
            render->clearViewport(imaging::Color4f(0.F, 0.F, 0.F, 1.F));
            render->setClipRectEnabled(true);
            drawEvent(widget, geometry, event);
            render->setClipRectEnabled(false);
            render->end();

            glPixelStorei(GL_PACK_ALIGNMENT, 1);
            glPixelStorei(GL_PACK_SWAP_BYTES, 0);
            glReadPixels(
                geometry.min.x,
                displaySize.h - geometry.min.y - size.h,
                size.w,
                size.h,
                gl::getReadPixelsFormat(info.pixelType),
                gl::getReadPixelsType(info.pixelType),
                out->getData());

            auto flipped = imaging::Image::create(info);
            for (size_t y = 0; y < size.h; ++y)
            {
                memcpy(
                    flipped->getData() + y * size.w * 4,
                    out->getData() + (size.h - 1 - y) * size.w * 4,
                    size.w * 4);
            }
            return flipped;
        }
    }
}
