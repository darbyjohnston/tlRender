// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlUI/IWidget.h>

namespace tl
{
    namespace ui
    {
        void IWidget::_init(
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
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
                    parent->_children.erase(i);
                }
            }
            _parent = value;
            if (value)
            {
                value->_children.push_back(
                    std::static_pointer_cast<IWidget>(shared_from_this()));
            }
        }

        const std::list<std::shared_ptr<IWidget> >& IWidget::getChildren() const
        {
            return _children;
        }

        void IWidget::sizeHint(const SizeHintData& data)
        {
            for (const auto& child : _children)
            {
                child->sizeHint(data);
            }
        }

        const math::Vector2i& IWidget::getSizeHint() const
        {
            return _sizeHint;
        }

        const math::BBox2i& IWidget::getGeometry() const
        {
            return _geometry;
        }

        void IWidget::setGeometry(const math::BBox2i& value)
        {
            _geometry = value;
        }

        void IWidget::draw(const DrawData& data)
        {
            for (const auto& child : _children)
            {
                child->draw(data);
            }
        }
    }
}
