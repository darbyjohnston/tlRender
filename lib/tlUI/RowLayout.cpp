// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlUI/RowLayout.h>

namespace tl
{
    namespace ui
    {
        void RowLayout::_init(
            Orientation orientation,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init(context, parent);

            _orientation = orientation;
        }

        RowLayout::RowLayout()
        {}

        RowLayout::~RowLayout()
        {}

        std::shared_ptr<RowLayout> RowLayout::create(
            Orientation orientation,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<RowLayout>(new RowLayout);
            out->_init(orientation, context, parent);
            return out;
        }

        void RowLayout::sizeHint(const SizeHintData& data)
        {
            IWidget::sizeHint(data);
            _sizeHint.x = 0;
            _sizeHint.y = 0;
            for (const auto& child : _children)
            {
                const math::Vector2i& sizeHint = child->getSizeHint();
                switch (_orientation)
                {
                case Orientation::Horizontal:
                    _sizeHint.x += sizeHint.x;
                    break;
                case Orientation::Vertical:
                    _sizeHint.y += sizeHint.y;
                    break;
                }
            }
        }

        void RowLayout::setGeometry(const math::BBox2i& value)
        {
            math::Vector2i pos = _geometry.min;
            for (const auto& child : _children)
            {
                const math::Vector2i& sizeHint = child->getSizeHint();
                child->setGeometry(math::BBox2i(pos.x, pos.y, sizeHint.x, sizeHint.y));
                switch (_orientation)
                {
                case Orientation::Horizontal:
                    pos.x += sizeHint.x;
                    break;
                case Orientation::Vertical:
                    pos.y += sizeHint.y;
                    break;
                }
            }
        }
    }
}
