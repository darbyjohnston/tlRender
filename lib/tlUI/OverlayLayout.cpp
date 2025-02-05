// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
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
            const std::shared_ptr<dtk::Context>& context,
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
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<OverlayLayout>(new OverlayLayout);
            out->_init(context, parent);
            return out;
        }

        void OverlayLayout::setMarginRole(SizeRole value)
        {
            DTK_P();
            if (value == p.marginRole)
                return;
            p.marginRole = value;
            p.size.sizeInit = true;
            _updates |= Update::Size;
            _updates |= Update::Draw;
        }

        void OverlayLayout::setGeometry(const dtk::Box2I& value)
        {
            IWidget::setGeometry(value);
            DTK_P();
            const dtk::Box2I g = margin(_geometry, -p.size.margin);
            _childrenClipRect = g;
            for (const auto& child : _children)
            {
                child->setGeometry(g);
            }
        }

        void OverlayLayout::sizeHintEvent(const SizeHintEvent& event)
        {
            const bool displayScaleChanged = event.displayScale != _displayScale;
            IWidget::sizeHintEvent(event);
            DTK_P();

            if (displayScaleChanged || p.size.sizeInit)
            {
                p.size.margin = event.style->getSizeRole(p.marginRole, _displayScale);
            }
            p.size.sizeInit = false;

            _sizeHint = dtk::Size2I();
            for (const auto& child : _children)
            {
                const dtk::Size2I& sizeHint = child->getSizeHint();
                _sizeHint.w = std::max(_sizeHint.w, sizeHint.w);
                _sizeHint.h = std::max(_sizeHint.h, sizeHint.h);
            }
            _sizeHint.w += p.size.margin * 2;
            _sizeHint.h += p.size.margin * 2;
        }
    }
}
