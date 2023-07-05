// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlTimelineUI/IItem.h>

#include <tlCore/Error.h>
#include <tlCore/String.h>
#include <tlCore/StringFormat.h>

namespace tl
{
    namespace timelineui
    {
        ItemOptions::ItemOptions()
        {
            colors[ColorRole::InOut] = imaging::Color4f(1.F, .7F, .2F, .1F);
            colors[ColorRole::VideoCache] = imaging::Color4f(.2F, .4F, .4F);
            colors[ColorRole::AudioCache] = imaging::Color4f(.3F, .25F, .4F);
            colors[ColorRole::VideoClip] = imaging::Color4f(.2F, .4F, .4F);
            colors[ColorRole::VideoGap] = imaging::Color4f(.25F, .31F, .31F);
            colors[ColorRole::AudioClip] = imaging::Color4f(.3F, .25F, .4F);
            colors[ColorRole::AudioGap] = imaging::Color4f(.25F, .24F, .3F);
            colors[ColorRole::Transition] = imaging::Color4f(.4F, .3F, .3F);
        }

        bool ItemOptions::operator == (const ItemOptions& other) const
        {
            return
                cacheDisplay == other.cacheDisplay &&
                colors == other.colors &&
                clipRectScale == other.clipRectScale &&
                thumbnails == other.thumbnails &&
                thumbnailHeight == other.thumbnailHeight &&
                waveformHeight == other.waveformHeight &&
                thumbnailFade == other.thumbnailFade;
        }

        bool ItemOptions::operator != (const ItemOptions& other) const
        {
            return !(*this == other);
        }

        struct IItem::Private
        {
            std::shared_ptr<observer::ValueObserver<bool> > timeUnitsObserver;
        };

        void IItem::_init(
            const std::string& name,
            const ItemData& data,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init(name, context, parent);
            TLRENDER_P();

            _data = data;

            p.timeUnitsObserver = observer::ValueObserver<bool>::create(
                data.timeUnitsModel->observeTimeUnitsChanged(),
                [this](bool)
                {
                    _timeUnitsUpdate();
                });
        }

        IItem::IItem() :
            _p(new Private)
        {}

        IItem::~IItem()
        {}

        void IItem::setScale(double value)
        {
            if (value == _scale)
                return;
            _scale = value;
            _updates |= ui::Update::Size;
            _updates |= ui::Update::Draw;
        }

        void IItem::setOptions(const ItemOptions& value)
        {
            if (value == _options)
                return;
            _options = value;
            _updates |= ui::Update::Size;
            _updates |= ui::Update::Draw;
        }

        math::BBox2i IItem::_getClipRect(
            const math::BBox2i& value,
            double scale)
        {
            math::BBox2i out;
            const math::Vector2i c = value.getCenter();
            out.min.x = (value.min.x - c.x) * scale + c.x;
            out.min.y = (value.min.y - c.y) * scale + c.y;
            out.max.x = (value.max.x - c.x) * scale + c.x;
            out.max.y = (value.max.y - c.y) * scale + c.y;
            return out;
        }

        std::string IItem::_durationLabel(const otime::RationalTime& value)
        {
            const otime::RationalTime rescaled = value.rescaled_to(_data.speed);
            return string::Format("{0}").
                arg(_data.timeUnitsModel->getLabel(rescaled));
        }

        void IItem::_timeUnitsUpdate()
        {}
    }
}
