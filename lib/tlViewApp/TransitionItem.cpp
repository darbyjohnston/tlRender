// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlViewApp/TransitionItem.h>

#include <tlCore/StringFormat.h>

namespace tl
{
    namespace view
    {
        struct TransitionItem::Private
        {
        };

        void TransitionItem::_init(
            otio::Transition* transition,
            const std::shared_ptr<IGraphicsItem>& parent)
        {
            IGraphicsItem::_init(parent);

            TLRENDER_P();

            _type = "Transition";
            _name = transition->name();
            _duration = transition->duration();
        }

        TransitionItem::TransitionItem() :
            _p(new Private)
        {}

        TransitionItem::~TransitionItem()
        {}

        std::shared_ptr<TransitionItem> TransitionItem::create(
            otio::Transition* transition,
            const std::shared_ptr<IGraphicsItem>&parent)
        {
            auto out = std::shared_ptr<TransitionItem>(new TransitionItem);
            out->_init(transition, parent);
            return out;
        }

        math::Vector2i TransitionItem::getSize(
            const std::shared_ptr<imaging::FontSystem>& fontSystem) const
        {
            return math::Vector2i(
                0.0,
                itemTitleFontInfo.size + itemMargin * 2 + itemBorder * 2);
        }

        void TransitionItem::draw(
            const math::BBox2i& bbox,
            const std::shared_ptr<imaging::FontSystem>& fontSystem,
            const std::shared_ptr<timeline::IRender>&render)
        {}
    }
}
