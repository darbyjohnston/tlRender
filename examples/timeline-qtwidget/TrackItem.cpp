// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include "TrackItem.h"

#include "ClipItem.h"
#include "GapItem.h"

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
                const std::shared_ptr<system::Context>& context)
            {
                BaseItem::_init(itemData, context);

                _timeRange = track->trimmed_range();

                for (const auto& child : track->children())
                {
                    if (auto clip = dynamic_cast<otio::Clip*>(child.value))
                    {
                        auto clipItem = ClipItem::create(clip, _itemData, context);
                        _children.push_back(clipItem);
                        const auto timeRangeOpt = track->trimmed_range_of_child(clip);
                        if (timeRangeOpt.has_value())
                        {
                            _timeRanges[clipItem] = timeRangeOpt.value();
                        }
                    }
                    else if (auto gap = dynamic_cast<otio::Gap*>(child.value))
                    {
                        auto gapItem = GapItem::create(gap, _itemData, context);
                        _children.push_back(gapItem);
                        const auto timeRangeOpt = track->trimmed_range_of_child(gap);
                        if (timeRangeOpt.has_value())
                        {
                            _timeRanges[gapItem] = timeRangeOpt.value();
                        }
                    }
                }

                _label = _nameLabel(track->kind(), track->name());
                _durationLabel = BaseItem::_durationLabel(_timeRange.duration());
            }

            std::shared_ptr<TrackItem> TrackItem::create(
                const otio::Track* track,
                const ItemData& itemData,
                const std::shared_ptr<system::Context>& context)
            {
                auto out = std::shared_ptr<TrackItem>(new TrackItem);
                out->_init(track, itemData, context);
                return out;
            }

            TrackItem::~TrackItem()
            {}

            void TrackItem::preLayout()
            {
                int childrenHeight = 0;
                for (const auto& child : _children)
                {
                    const auto& sizeHint = child->sizeHint();
                    childrenHeight = std::max(childrenHeight, sizeHint.y);
                }

                _sizeHint = math::Vector2i(
                    _timeRange.duration().rescaled_to(1.0).value() * _scale,
                    _itemData.margin +
                    _itemData.fontMetrics.lineHeight +
                    _itemData.margin +
                    childrenHeight);
            }

            void TrackItem::layout(const math::BBox2i& geometry)
            {
                BaseItem::layout(geometry);
                for (const auto& child : _children)
                {
                    const auto i = _timeRanges.find(child);
                    if (i != _timeRanges.end())
                    {
                        const auto& sizeHint = child->sizeHint();
                        child->layout(math::BBox2i(
                            _geometry.min.x +
                            i->second.start_time().rescaled_to(1.0).value() * _scale,
                            _geometry.min.y +
                            _itemData.margin +
                            _itemData.fontMetrics.lineHeight +
                            _itemData.margin,
                            sizeHint.x,
                            sizeHint.y));
                    }
                }
            }

            void TrackItem::render(
                const std::shared_ptr<timeline::IRender>& render,
                const math::BBox2i& viewport,
                float devicePixelRatio)
            {
                BaseItem::render(render, viewport, devicePixelRatio);
                if (_geometry.intersects(viewport))
                {
                    auto fontInfo = _itemData.fontInfo;
                    fontInfo.size *= devicePixelRatio;
                    render->drawText(
                        _itemData.fontSystem->getGlyphs(_label, fontInfo),
                        math::Vector2i(
                            _geometry.min.x +
                            _itemData.margin,
                            _geometry.min.y +
                            _itemData.margin +
                            _itemData.fontMetrics.ascender) * devicePixelRatio,
                        imaging::Color4f(.9F, .9F, .9F));
                    math::Vector2i textSize = _itemData.fontSystem->measure(_durationLabel, _itemData.fontInfo);
                    render->drawText(
                        _itemData.fontSystem->getGlyphs(_durationLabel, fontInfo),
                        math::Vector2i(
                            _geometry.max.x -
                            _itemData.margin -
                            textSize.x,
                            _geometry.min.y +
                            _itemData.margin +
                            _itemData.fontMetrics.ascender) * devicePixelRatio,
                        imaging::Color4f(.9F, .9F, .9F));
                }
            }

            std::string TrackItem::_nameLabel(
                const std::string& kind,
                const std::string& name)
            {
                return !name.empty() && name != "Track" ?
                    string::Format("{0} Track: {1}").arg(kind).arg(name) :
                    string::Format("{0} Track").arg(kind);
            }
        }
    }
}
