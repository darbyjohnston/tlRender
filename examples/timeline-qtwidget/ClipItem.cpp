// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include "ClipItem.h"

#include <QPainter>

namespace tl
{
    namespace examples
    {
        namespace timeline_qtwidget
        {
            void  ClipItem::_init(
                const otio::Clip* clip,
                const ItemData& itemData,
                const std::shared_ptr<system::Context>& context)
            {
                BaseItem::_init(itemData, context);

                auto rangeOpt = clip->trimmed_range_in_parent();
                if (rangeOpt.has_value())
                {
                    _timeRange = rangeOpt.value();
                }

                _label = _nameLabel(clip->name());
                _durationLabel = BaseItem::_durationLabel(_timeRange.duration());
                _startLabel = _timeLabel(_timeRange.start_time());
                _endLabel = _timeLabel(_timeRange.end_time_inclusive());
            }

            ClipItem::~ClipItem()
            {}

            std::shared_ptr<ClipItem>  ClipItem::create(
                const otio::Clip* clip,
                const ItemData& itemData,
                const std::shared_ptr<system::Context>& context)
            {
                auto out = std::shared_ptr<ClipItem>(new ClipItem);
                out->_init(clip, itemData, context);
                return out;
            }

            void ClipItem::preLayout()
            {
                _sizeHint = math::Vector2i(
                    _timeRange.duration().rescaled_to(1.0).value() * _scale,
                    _itemData.border +
                    _itemData.margin +
                    _itemData.fontMetrics.lineHeight +
                    _itemData.spacing +
                    _itemData.fontMetrics.lineHeight +
                    _itemData.margin +
                    _itemData.border);
            }

            void ClipItem::render(
                const std::shared_ptr<timeline::IRender>& render,
                const math::BBox2i& viewport,
                float devicePixelRatio)
            {
                BaseItem::render(render, viewport, devicePixelRatio);
                if (_geometry.intersects(viewport))
                {
                    render->drawRect(
                        _geometry * devicePixelRatio,
                        imaging::Color4f(.35, .45, .35));
                    render->drawRect(
                        _geometry.margin(-_itemData.border) * devicePixelRatio,
                        imaging::Color4f(.25, .35, .25));

                    auto fontInfo = _itemData.fontInfo;
                    fontInfo.size *= devicePixelRatio;
                    render->drawText(
                        _itemData.fontSystem->getGlyphs(_label, fontInfo),
                        math::Vector2i(
                            _geometry.min.x +
                            _itemData.border +
                            _itemData.margin,
                            _geometry.min.y +
                            _itemData.border +
                            _itemData.margin +
                            _itemData.fontMetrics.ascender) * devicePixelRatio,
                        imaging::Color4f(.9F, .9F, .9F));
                    render->drawText(
                        _itemData.fontSystem->getGlyphs(_startLabel, fontInfo),
                        math::Vector2i(
                            _geometry.min.x +
                            _itemData.border +
                            _itemData.margin,
                            _geometry.min.y +
                            _itemData.border +
                            _itemData.margin +
                            _itemData.fontMetrics.lineHeight +
                            _itemData.spacing +
                            _itemData.fontMetrics.ascender) * devicePixelRatio,
                        imaging::Color4f(.9F, .9F, .9F));

                    math::Vector2i textSize = _itemData.fontSystem->measure(_durationLabel, _itemData.fontInfo);
                    render->drawText(
                        _itemData.fontSystem->getGlyphs(_durationLabel, fontInfo),
                        math::Vector2i(
                            _geometry.max.x -
                            _itemData.border -
                            _itemData.margin -
                            textSize.x,
                            _geometry.min.y +
                            _itemData.border +
                            _itemData.margin +
                            _itemData.fontMetrics.ascender) * devicePixelRatio,
                        imaging::Color4f(.9F, .9F, .9F));
                    textSize = _itemData.fontSystem->measure(_endLabel, _itemData.fontInfo);
                    render->drawText(
                        _itemData.fontSystem->getGlyphs(_endLabel, fontInfo),
                        math::Vector2i(
                            _geometry.max.x -
                            _itemData.border -
                            _itemData.margin -
                            textSize.x,
                            _geometry.min.y +
                            _itemData.border +
                            _itemData.margin +
                            _itemData.fontMetrics.lineHeight +
                            _itemData.spacing +
                            _itemData.fontMetrics.ascender) * devicePixelRatio,
                        imaging::Color4f(.9F, .9F, .9F));
                }
            }

            std::string ClipItem::_nameLabel(const std::string& name)
            {
                return !name.empty() ?
                    name :
                    std::string("Clip");
            }
        }
    }
}
