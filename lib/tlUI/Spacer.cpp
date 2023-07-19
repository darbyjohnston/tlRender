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
            _p->spacingRole = value;
        }

        void Spacer::sizeHintEvent(const SizeHintEvent& event)
        {
            IWidget::sizeHintEvent(event);
            TLRENDER_P();
            _sizeHint = math::Vector2i();
            switch (p.orientation)
            {
            case Orientation::Horizontal:
                _sizeHint.x = event.style->getSizeRole(p.spacingRole, event.displayScale);
                break;
            case Orientation::Vertical:
                _sizeHint.y = event.style->getSizeRole(p.spacingRole, event.displayScale);
                break;
            default: break;
            }
        }
    }
}
