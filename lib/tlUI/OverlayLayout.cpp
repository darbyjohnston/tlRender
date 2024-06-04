// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlUI/OverlayLayout.h>

namespace tl
{
    namespace ui
    {
        struct OverlayLayout::Private
        {
            SizeRole marginRole = SizeRole::None;

            struct SizeData
            {
                bool sizeInit = true;
                int margin = 0;
            };
            SizeData size;
        };

        void OverlayLayout::_init(
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init("tl::ui::OverlayLayout", context, parent);
        }

        OverlayLayout::OverlayLayout() :
            _p(new Private)
        {}

        OverlayLayout::~OverlayLayout()
        {}

        std::shared_ptr<OverlayLayout> OverlayLayout::create(
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<OverlayLayout>(new OverlayLayout);
            out->_init(context, parent);
            return out;
        }

        void OverlayLayout::setMarginRole(SizeRole value)
        {
            TLRENDER_P();
            if (value == p.marginRole)
                return;
            p.marginRole = value;
            p.size.sizeInit = true;
            _updates |= Update::Size;
            _updates |= Update::Draw;
        }

        void OverlayLayout::setGeometry(const math::Box2i& value)
        {
            IWidget::setGeometry(value);
            TLRENDER_P();
            const math::Box2i g = _geometry.margin(-p.size.margin);
            for (const auto& child : _children)
            {
                child->setGeometry(g);
            }
        }

        math::Box2i OverlayLayout::getChildrenClipRect() const
        {
            return _geometry.margin(-_p->size.margin);
        }

        void OverlayLayout::sizeHintEvent(const SizeHintEvent& event)
        {
            const bool displayScaleChanged = event.displayScale != _displayScale;
            IWidget::sizeHintEvent(event);
            TLRENDER_P();

            if (displayScaleChanged || p.size.sizeInit)
            {
                p.size.margin = event.style->getSizeRole(p.marginRole, _displayScale);
            }
            p.size.sizeInit = false;

            _sizeHint = math::Size2i();
            for (const auto& child : _children)
            {
                const math::Size2i& sizeHint = child->getSizeHint();
                _sizeHint.w = std::max(_sizeHint.w, sizeHint.w);
                _sizeHint.h = std::max(_sizeHint.h, sizeHint.h);
            }
            _sizeHint.w += p.size.margin * 2;
            _sizeHint.h += p.size.margin * 2;
        }
    }
}
