// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlViewApp/TrackItem.h>

#include <tlViewApp/Util.h>

#include <tlCore/StringFormat.h>

namespace tl
{
    namespace view
    {
        struct TrackItem::Private
        {
        };

        void TrackItem::_init(
            otio::Track* track,
            const std::shared_ptr<IGraphicsItem>& parent)
        {
            IGraphicsItem::_init(parent);

            TLRENDER_P();

            _type = "Track";
            _name = track->name();
            _duration = track->duration();
        }

        TrackItem::TrackItem() :
            _p(new Private)
        {}

        TrackItem::~TrackItem()
        {}

        std::shared_ptr<TrackItem> TrackItem::create(
            otio::Track* track,
            const std::shared_ptr<IGraphicsItem>&parent)
        {
            auto out = std::shared_ptr<TrackItem>(new TrackItem);
            out->_init(track, parent);
            return out;
        }

        math::Vector2i TrackItem::getSize(
            const std::shared_ptr<imaging::FontSystem>& fontSystem) const
        {
            imaging::FontMetrics fontMetrics = fontSystem->getMetrics(itemTitleFontInfo);
            math::Vector2i size = math::Vector2i(
                _duration.rescaled_to(1.0).value() * secondsSize,
                fontMetrics.lineHeight + itemMargin * 2 + itemBorder * 2);
            return size;
        }

        void TrackItem::draw(
            const math::BBox2i& bbox,
            const std::shared_ptr<imaging::FontSystem>& fontSystem,
            const std::shared_ptr<timeline::IRender>&render)
        {
            render->drawRect(bbox, imaging::Color4f(.8F, .8F, .8F));
            drawBorder(bbox, itemBorder, imaging::Color4f(.1F, .1F, .1F), render);

            const math::BBox2i marginRect = bbox.margin(-(itemMargin + itemBorder));

            math::BBox2i textRect = marginRect;
            imaging::FontMetrics fontMetrics = fontSystem->getMetrics(itemSmallFontInfo);
            math::Vector2i textSize = fontSystem->measure(_type, itemSmallFontInfo);
            auto textGlyphs = fontSystem->getGlyphs(_type, itemSmallFontInfo);
            math::Vector2i textPos(
                textRect.min.x,
                textRect.min.y + fontMetrics.ascender - 1);
            render->drawText(textGlyphs, textPos, imaging::Color4f(.1F, .1F, .1F));

            fontMetrics = fontSystem->getMetrics(itemTitleFontInfo);
            textSize = fontSystem->measure(_name, itemTitleFontInfo);
            textGlyphs = fontSystem->getGlyphs(_name, itemTitleFontInfo);
            textPos = math::Vector2i(
                textRect.min.x + textRect.w() / 2 - textSize.x / 2,
                textRect.min.y + textRect.h() / 2 - textSize.y / 2 + fontMetrics.ascender - 1);
            render->drawText(textGlyphs, textPos, imaging::Color4f(.1F, .1F, .1F));
        }
    }
}
