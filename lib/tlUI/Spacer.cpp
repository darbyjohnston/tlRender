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
            SizeRole spacingRole = SizeRole::Spacing;
        };

        void Spacer::_init(
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init("tl::ui::Spacer", context, parent);
        }

        Spacer::Spacer() :
            _p(new Private)
        {}

        Spacer::~Spacer()
        {}

        std::shared_ptr<Spacer> Spacer::create(
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<Spacer>(new Spacer);
            out->_init(context, parent);
            return out;
        }

        void Spacer::setSpacingRole(SizeRole value)
        {
            _p->spacingRole = value;
        }

        void Spacer::sizeHintEvent(const SizeHintEvent& event)
        {
            TLRENDER_P();
            _sizeHint.x = _sizeHint.y = event.style->getSizeRole(p.spacingRole);
        }
    }
}
