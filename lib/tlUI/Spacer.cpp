// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlUI/Spacer.h>

namespace tl
{
    namespace ui
    {
        struct Spacer::Private
        {
            Orientation orientation = Orientation::Horizontal;
            SizeRole spacingRole = SizeRole::Spacing;

            struct SizeData
            {
                bool sizeInit = true;
                int size = 0;
            };
            SizeData size;
        };

        void Spacer::_init(
            Orientation orientation,
            const std::shared_ptr<system::Context>& context,
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
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<Spacer>(new Spacer);
            out->_init(orientation, context, parent);
            return out;
        }

        void Spacer::setSpacingRole(SizeRole value)
        {
            TLRENDER_P();
            if (value == p.spacingRole)
                return;
            p.spacingRole = value;
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
                p.size.size = event.style->getSizeRole(p.spacingRole, _displayScale);
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
