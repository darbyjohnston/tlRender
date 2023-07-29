// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlUI/Icon.h>

#include <tlUI/LayoutUtil.h>

#include <tlCore/String.h>

namespace tl
{
    namespace ui
    {
        struct Icon::Private
        {
            std::string icon;
            std::shared_ptr<imaging::Image> iconImage;
            float iconScale = 1.F;
            bool iconInit = false;
            std::future<std::shared_ptr<imaging::Image> > iconFuture;
            SizeRole marginRole = SizeRole::None;

            struct SizeData
            {
                int margin = 0;
            };
            SizeData size;

            struct DrawData
            {
            };
            DrawData draw;
        };

        void Icon::_init(
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init("tl::ui::Icon", context, parent);
            _hAlign = HAlign::Left;
        }

        Icon::Icon() :
            _p(new Private)
        {}

        Icon::~Icon()
        {}

        std::shared_ptr<Icon> Icon::create(
            const std::shared_ptr<system::Context>&context,
            const std::shared_ptr<IWidget>&parent)
        {
            auto out = std::shared_ptr<Icon>(new Icon);
            out->_init(context, parent);
            return out;
        }

        std::shared_ptr<Icon> Icon::create(
            const std::string& icon,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<Icon>(new Icon);
            out->_init(context, parent);
            out->setIcon(icon);
            return out;
        }

        void Icon::setIcon(const std::string& value)
        {
            TLRENDER_P();
            if (value == p.icon)
                return;
            p.icon = value;
            p.iconImage.reset();
            p.iconInit = true;
            _updates |= Update::Size;
            _updates |= Update::Draw;
        }

        void Icon::setMarginRole(SizeRole value)
        {
            TLRENDER_P();
            if (value == p.marginRole)
                return;
            p.marginRole = value;
            _updates |= Update::Size;
            _updates |= Update::Draw;
        }

        void Icon::tickEvent(
            bool parentsVisible,
            bool parentsEnabled,
            const TickEvent& event)
        {
            IWidget::tickEvent(parentsVisible, parentsEnabled, event);
            TLRENDER_P();
            if (event.displayScale != p.iconScale)
            {
                p.iconImage.reset();
                p.iconScale = event.displayScale;
                p.iconInit = true;
                p.iconFuture = std::future<std::shared_ptr<imaging::Image> >();
            }
            if (!p.icon.empty() && p.iconInit)
            {
                p.iconInit = false;
                p.iconFuture = event.iconLibrary->request(p.icon, event.displayScale);
            }
            if (p.iconFuture.valid() &&
                p.iconFuture.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
            {
                p.iconImage = p.iconFuture.get();
                _updates |= Update::Size;
                _updates |= Update::Draw;
            }
        }

        void Icon::sizeHintEvent(const SizeHintEvent& event)
        {
            IWidget::sizeHintEvent(event);
            TLRENDER_P();

            p.size.margin = event.style->getSizeRole(p.marginRole, event.displayScale);

            _sizeHint = math::Vector2i();
            if (p.iconImage)
            {
                _sizeHint.x = p.iconImage->getWidth();
                _sizeHint.y = p.iconImage->getHeight();
            }
        }

        void Icon::clipEvent(
            const math::BBox2i& clipRect,
            bool clipped,
            const ClipEvent& event)
        {
            IWidget::clipEvent(clipRect, clipped, event);
            TLRENDER_P();
            if (clipped)
            {
            }
        }

        void Icon::drawEvent(
            const math::BBox2i& drawRect,
            const DrawEvent& event)
        {
            IWidget::drawEvent(drawRect, event);
            TLRENDER_P();
            const math::BBox2i g = _geometry.margin(-p.size.margin);
            if (p.iconImage)
            {
                const imaging::Size& iconSize = p.iconImage->getSize();
                event.render->drawImage(
                    p.iconImage,
                    math::BBox2i(
                        g.x() + g.w() / 2 - iconSize.w / 2,
                        g.y() + g.h() / 2 - iconSize.h / 2,
                        iconSize.w,
                        iconSize.h),
                    event.style->getColorRole(ColorRole::Text));
            }
        }
    }
}
