// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlViewApp/IGraphicsItem.h>

namespace tl
{
    namespace view
    {
        void IGraphicsItem::_init(
            const std::shared_ptr<IGraphicsItem>& parent)
        {
            _parent = parent;
            if (parent)
            {
                parent->_children.push_back(shared_from_this());
            }
        }

        IGraphicsItem::IGraphicsItem()
        {}

        IGraphicsItem::~IGraphicsItem()
        {}

        const std::weak_ptr<IGraphicsItem>& IGraphicsItem::getParent() const
        {
            return _parent;
        }

        std::list<std::shared_ptr<IGraphicsItem> > IGraphicsItem::getChildren() const
        {
            return _children;
        }

        const otime::RationalTime& IGraphicsItem::getDuration() const
        {
            return _duration;
        }
    }
}
