// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlUI/Spacer.h>

namespace tl
{
    namespace ui
    {
        struct Spacer::Private
        {
            Orientation orientation = Orientation::Horizontal;
            SizeRole sizeRole = SizeRole::Spacing;

            struct SizeData
            {
                bool sizeInit = true;
                int size = 0;
            };
            SizeData size;
        };

        void Spacer::_init(
            Orientation orientation,
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init("tl::ui::Spacer", context, parent);
            TLRENDER_P();
            p.orientation = orientation;
        }

        Spacer::Spacer() :
            _p(new Private)
        {}

        Spacer::~Spacer()
        {}

        std::shared_ptr<Spacer> Spacer::create(
            Orientation orientation,
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<Spacer>(new Spacer);
            out->_init(orientation, context, parent);
            return out;
        }

        void Spacer::setSizeRole(SizeRole value)
        {
            TLRENDER_P();
            if (value == p.sizeRole)
                return;
            p.sizeRole = value;
            p.size.sizeInit = true;
            _updates |= Update::Size;
        }

        void Spacer::sizeHintEvent(const SizeHintEvent& event)
        {
            const bool displayScaleChanged = event.displayScale != _displayScale;
            IWidget::sizeHintEvent(event);
            TLRENDER_P();

            if (displayScaleChanged || p.size.sizeInit)
            {
                p.size.size = event.style->getSizeRole(p.sizeRole, _displayScale);
            }
            p.size.sizeInit = false;

            _sizeHint = math::Size2i();
            switch (p.orientation)
            {
            case Orientation::Horizontal:
                _sizeHint.w = p.size.size;
                break;
            case Orientation::Vertical:
                _sizeHint.h = p.size.size;
                break;
            default: break;
            }
        }
    }
}
