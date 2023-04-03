// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include "TrackItem.h"

#include "AudioClipItem.h"
#include "AudioGapItem.h"
#include "VideoClipItem.h"
#include "VideoGapItem.h"

#include <tlCore/StringFormat.h>

namespace tl
{
    namespace examples
    {
        namespace timeline_qtwidget
        {
            void TrackItem::_init(
                const otio::Track* track,
                const ItemData& itemData,
                const std::shared_ptr<system::Context>& context,
                const std::shared_ptr<IWidget>& parent)
            {
                IItem::_init("TrackItem", itemData, context, parent);

                if (otio::Track::Kind::video == track->kind())
                {
                    _trackType = TrackType::Video;
                }
                else if (otio::Track::Kind::audio == track->kind())
                {
                    _trackType = TrackType::Audio;
                }

                _timeRange = track->trimmed_range();

                for (const auto& child : track->children())
                {
                    if (auto clip = dynamic_cast<otio::Clip*>(child.value))
                    {
                        std::shared_ptr<IItem> clipItem;
                        switch (_trackType)
                        {
                        case TrackType::Video:
                            clipItem = VideoClipItem::create(
                                clip,
                                itemData,
                                context,
                                shared_from_this());
                            break;
                        case TrackType::Audio:
                            clipItem = AudioClipItem::create(
                                clip,
                                itemData,
                                context,
                                shared_from_this());
                            break;
                        }
                        const auto timeRangeOpt = track->trimmed_range_of_child(clip);
                        if (timeRangeOpt.has_value())
                        {
                            _childTimeRanges[clipItem] = timeRangeOpt.value();
                        }
                    }
                    else if (auto gap = dynamic_cast<otio::Gap*>(child.value))
                    {
                        std::shared_ptr<IItem> gapItem;
                        switch (_trackType)
                        {
                        case TrackType::Video:
                            gapItem = VideoGapItem::create(
                                gap,
                                itemData,
                                context,
                                shared_from_this());
                            break;
                        case TrackType::Audio:
                            gapItem = AudioGapItem::create(
                                gap,
                                itemData,
                                context,
                                shared_from_this());
                            break;
                        }
                        const auto timeRangeOpt = track->trimmed_range_of_child(gap);
                        if (timeRangeOpt.has_value())
                        {
                            _childTimeRanges[gapItem] = timeRangeOpt.value();
                        }
                    }
                }
            }

            std::shared_ptr<TrackItem> TrackItem::create(
                const otio::Track* track,
                const ItemData& itemData,
                const std::shared_ptr<system::Context>& context,
                const std::shared_ptr<IWidget>& parent)
            {
                auto out = std::shared_ptr<TrackItem>(new TrackItem);
                out->_init(track, itemData, context, parent);
                return out;
            }

            TrackItem::~TrackItem()
            {}

            void TrackItem::setGeometry(const math::BBox2i& value)
            {
                IItem::setGeometry(value);
                for (auto child : _children)
                {
                    if (auto item = std::dynamic_pointer_cast<IItem>(child))
                    {
                        const auto i = _childTimeRanges.find(item);
                        if (i != _childTimeRanges.end())
                        {
                            const math::Vector2i& sizeHint = child->getSizeHint();
                            math::BBox2i bbox(
                                _geometry.min.x +
                                i->second.start_time().rescaled_to(1.0).value() * _options.scale,
                                _geometry.min.y,
                                sizeHint.x,
                                sizeHint.y);
                            child->setGeometry(bbox);
                        }
                    }
                }
            }

            void TrackItem::sizeEvent(const ui::SizeEvent& event)
            {
                IItem::sizeEvent(event);

                _margin = event.style->getSizeRole(ui::SizeRole::MarginSmall) * event.contentScale;

                int childrenHeight = 0;
                for (const auto& child : _children)
                {
                    childrenHeight = std::max(childrenHeight, child->getSizeHint().y);
                }

                _sizeHint = math::Vector2i(
                    _timeRange.duration().rescaled_to(1.0).value() * _options.scale,
                    childrenHeight);
            }

            void TrackItem::drawEvent(const ui::DrawEvent& event)
            {
                IItem::drawEvent(event);
            }
        }
    }
}
