// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlUI/Divider.h>

namespace tl
{
    namespace ui
    {
        struct Divider::Private
        {
            struct SizeData
            {
                bool sizeInit = true;
                int size = 0;
            };
            SizeData size;
        };

        void Divider::_init(
            Orientation orientation,
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init("tl::ui::Divider", context, parent);
            setBackgroundRole(ColorRole::Border);
            switch (orientation)
            {
            case Orientation::Horizontal:
                setVStretch(Stretch::Expanding);
                break;
            case Orientation::Vertical:
                setHStretch(Stretch::Expanding);
                break;
            }
        }

        Divider::Divider() :
            _p(new Private)
        {}

        Divider::~Divider()
        {}

        std::shared_ptr<Divider> Divider::create(
            Orientation orientation,
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<Divider>(new Divider);
            out->_init(orientation, context, parent);
            return out;
        }

        void Divider::sizeHintEvent(const SizeHintEvent& event)
        {
            const bool displayScaleChanged = event.displayScale != _displayScale;
            IWidget::sizeHintEvent(event);
            TLRENDER_P();

            if (displayScaleChanged || p.size.sizeInit)
            {
                p.size.size = event.style->getSizeRole(SizeRole::Border, _displayScale);
            }
            p.size.sizeInit = false;

            _sizeHint.w = _sizeHint.h = p.size.size;
        }
    }
}
