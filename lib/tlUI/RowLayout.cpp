// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlUI/RowLayout.h>

#include <set>

namespace tl
{
    namespace ui
    {
        struct RowLayout::Private
        {
            Orientation orientation = Orientation::Horizontal;
            SizeRole marginRole = SizeRole::None;
            SizeRole spacingRole = SizeRole::Spacing;

            struct SizeData
            {
                int margin = 0;
                int spacing = 0;
            };
            SizeData size;
        };

        void RowLayout::_init(
            Orientation orientation,
            const std::string& name,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init(name, context, parent);
            TLRENDER_P();
            p.orientation = orientation;
        }

        RowLayout::RowLayout() :
            _p(new Private)
        {}

        RowLayout::~RowLayout()
        {}

        std::shared_ptr<RowLayout> RowLayout::create(
            Orientation orientation,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<RowLayout>(new RowLayout);
            out->_init(orientation, "tl::ui::RowLayout", context, parent);
            return out;
        }

        void RowLayout::setMarginRole(SizeRole value)
        {
            _p->marginRole = value;
        }

        void RowLayout::setSpacingRole(SizeRole value)
        {
            _p->spacingRole = value;
        }

        void RowLayout::setGeometry(const math::BBox2i& value)
        {
            IWidget::setGeometry(value);
            TLRENDER_P();
            const math::BBox2i g = _geometry.margin(-p.size.margin);
            size_t expanding = 0;
            for (const auto& child : _children)
            {
                switch (p.orientation)
                {
                case Orientation::Horizontal:
                    if (Stretch::Expanding == child->getHStretch())
                    {
                        ++expanding;
                    }
                    break;
                case Orientation::Vertical:
                    if (Stretch::Expanding == child->getVStretch())
                    {
                        ++expanding;
                    }
                    break;
                }
            }
            const std::pair<int, int> extra(
                _geometry.w() - _sizeHint.x,
                _geometry.h() - _sizeHint.y);
            math::Vector2i pos = g.min;
            for (const auto& child : _children)
            {
                math::Vector2i size = child->getSizeHint();
                const bool last = child == _children.back();
                switch (p.orientation)
                {
                case Orientation::Horizontal:
                    size.y = g.h();
                    if (Stretch::Expanding == child->getHStretch())
                    {
                        size.x += extra.first / expanding;
                        if (last)
                        {
                            size.x += extra.first - (extra.first / expanding * expanding);
                        }
                    }
                    break;
                case Orientation::Vertical:
                    size.x = g.w();
                    if (Stretch::Expanding == child->getVStretch())
                    {
                        size.y += extra.second / expanding;
                        if (last)
                        {
                            size.y += extra.second - (extra.second / expanding * expanding);
                        }
                    }
                    break;
                }
                child->setGeometry(math::BBox2i(pos.x, pos.y, size.x, size.y));
                switch (p.orientation)
                {
                case Orientation::Horizontal:
                    pos.x += size.x + p.size.spacing;
                    break;
                case Orientation::Vertical:
                    pos.y += size.y + p.size.spacing;
                    break;
                }
            }
        }

        void RowLayout::sizeEvent(const SizeEvent& event)
        {
            IWidget::sizeEvent(event);
            TLRENDER_P();

            p.size.margin = event.style->getSizeRole(p.marginRole) * event.contentScale;
            p.size.spacing = event.style->getSizeRole(p.spacingRole) * event.contentScale;

            _sizeHint = math::Vector2i();
            for (const auto& child : _children)
            {
                const math::Vector2i& sizeHint = child->getSizeHint();
                switch (p.orientation)
                {
                case Orientation::Horizontal:
                    _sizeHint.x += sizeHint.x;
                    _sizeHint.y = std::max(_sizeHint.y, sizeHint.y);
                    break;
                case Orientation::Vertical:
                    _sizeHint.x = std::max(_sizeHint.x, sizeHint.x);
                    _sizeHint.y += sizeHint.y;
                    break;
                }
            }
            if (!_children.empty())
            {
                const size_t count = _children.size();
                switch (p.orientation)
                {
                case Orientation::Horizontal:
                    _sizeHint.x += p.size.spacing * (count - 1);
                    break;
                case Orientation::Vertical:
                    _sizeHint.y += p.size.spacing * (count - 1);
                    break;
                }
            }
            _sizeHint.x += p.size.margin * 2;
            _sizeHint.y += p.size.margin * 2;
        }

        void RowLayout::childAddedEvent(const ChildEvent&)
        {
            _updates |= Update::Size;
        }

        void RowLayout::childRemovedEvent(const ChildEvent&)
        {
            _updates |= Update::Size;
        }

        void HorizontalLayout::_init(
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            RowLayout::_init(
                Orientation::Horizontal,
                "tl::ui::HorizontalLayout",
                context,
                parent);
        }

        HorizontalLayout::HorizontalLayout()
        {}

        HorizontalLayout::~HorizontalLayout()
        {}

        std::shared_ptr<HorizontalLayout> HorizontalLayout::create(
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<HorizontalLayout>(new HorizontalLayout);
            out->_init(context, parent);
            return out;
        }

        void VerticalLayout::_init(
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            RowLayout::_init(
                Orientation::Vertical,
                "tl::ui::VerticalLayout",
                context,
                parent);
        }

        VerticalLayout::VerticalLayout()
        {}

        VerticalLayout::~VerticalLayout()
        {}

        std::shared_ptr<VerticalLayout> VerticalLayout::create(
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<VerticalLayout>(new VerticalLayout);
            out->_init(context, parent);
            return out;
        }
    }
}
