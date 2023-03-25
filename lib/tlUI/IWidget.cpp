// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlUI/IWidget.h>

namespace tl
{
    namespace ui
    {
        void IWidget::_init(
            const std::string& name,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            _context = context;
            _name = name;
            if (parent)
            {
                parent->_children.push_back(
                    std::static_pointer_cast<IWidget>(shared_from_this()));
            }
            _parent = parent;
        }

        IWidget::IWidget()
        {}

        IWidget::~IWidget()
        {}

        const std::string& IWidget::getName() const
        {
            return _name;
        }

        void IWidget::setName(const std::string& value)
        {
            _name = value;
        }
        
        void IWidget::setParent(const std::shared_ptr<IWidget>& value)
        {
            if (auto parent = _parent.lock())
            {
                auto i = std::find(
                    parent->_children.begin(),
                    parent->_children.end(),
                    shared_from_this());
                if (i != parent->_children.end())
                {
                    ChildEvent event;
                    event.child = *i;
                    parent->_children.erase(i);
                    parent->childRemovedEvent(event);
                    parent->_updates |= Update::Size;
                    parent->_updates |= Update::Draw;
                }
            }
            _parent = value;
            if (value)
            {
                value->_children.push_back(
                    std::static_pointer_cast<IWidget>(shared_from_this()));
                ChildEvent event;
                event.child = shared_from_this();
                value->childAddedEvent(event);
                value->_updates |= Update::Size;
                value->_updates |= Update::Draw;
            }
        }

        const std::list<std::shared_ptr<IWidget> >& IWidget::getChildren() const
        {
            return _children;
        }

        const math::Vector2i& IWidget::getSizeHint() const
        {
            return _sizeHint;
        }

        Stretch IWidget::getStretch(Orientation orientation) const
        {
            return Orientation::Horizontal == orientation ?
                _stretch.first :
                _stretch.second;
        }

        void IWidget::setStretch(Stretch value, Orientation orientation)
        {
            switch (orientation)
            {
            case Orientation::Horizontal:
                if (value != _stretch.first)
                {
                    _stretch.first = value;
                    _updates |= Update::Size;
                    _updates |= Update::Draw;
                }
                break;
            case Orientation::Vertical:
                if (value != _stretch.second)
                {
                    _stretch.second = value;
                    _updates |= Update::Size;
                    _updates |= Update::Draw;
                }
                break;
            }
        }

        const math::BBox2i& IWidget::getGeometry() const
        {
            return _geometry;
        }

        void IWidget::setGeometry(const math::BBox2i& value)
        {
            if (value == _geometry)
                return;
            _geometry = value;
            _updates |= Update::Size;
            _updates |= Update::Draw;
        }

        bool IWidget::isVisible() const
        {
            return _visible;
        }

        void IWidget::setVisible(bool value)
        {
            if (value == _visible)
                return;
            _visible = value;
            _updates |= Update::Size;
            _updates |= Update::Draw;
        }

        void IWidget::setBackgroundRole(ColorRole value)
        {
            if (value == _backgroundRole)
                return;
            _backgroundRole = value;
            _updates |= Update::Draw;
        }

        int IWidget::getUpdates() const
        {
            return _updates;
        }

        void IWidget::childAddedEvent(const ChildEvent&)
        {}

        void IWidget::childRemovedEvent(const ChildEvent&)
        {}

        void IWidget::tickEvent(const TickEvent&)
        {}

        void IWidget::sizeEvent(const SizeEvent&)
        {
            _updates &= ~static_cast<int>(Update::Size);
        }

        void IWidget::drawEvent(const DrawEvent& event)
        {
            _updates &= ~static_cast<int>(Update::Draw);
            if (_backgroundRole != ColorRole::None)
            {
                event.render->drawRect(
                    _geometry,
                    event.style->getColorRole(_backgroundRole));
            }
        }

        void IWidget::enterEvent()
        {}

        void IWidget::leaveEvent()
        {}

        void IWidget::mouseMoveEvent(const MouseMoveEvent&)
        {}

        void IWidget::mousePressEvent(const MouseClickEvent&)
        {}

        void IWidget::mouseReleaseEvent(const MouseClickEvent&)
        {}

        void IWidget::keyPressEvent(const KeyEvent&)
        {}

        void IWidget::keyReleaseEvent(const KeyEvent&)
        {}
    }
}
