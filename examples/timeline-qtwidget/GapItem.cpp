// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include "GapItem.h"

#include <QPainter>

namespace tl
{
    namespace examples
    {
        namespace timeline_qtwidget
        {
            void GapItem::_init(
                const otio::Gap* gap,
                const ItemData& itemData,
                const std::shared_ptr<system::Context>& context)
            {
                BaseItem::_init(itemData, context);

                auto rangeOpt = gap->trimmed_range_in_parent();
                if (rangeOpt.has_value())
                {
                    _timeRange = rangeOpt.value();
                }

                _label = _nameLabel(gap->name());
                _durationLabel = BaseItem::_durationLabel(gap->duration());
                _startLabel = _timeLabel(_timeRange.start_time());
                _endLabel = _timeLabel(_timeRange.end_time_inclusive());
            }

            GapItem::~GapItem()
            {}

            std::shared_ptr<GapItem> GapItem::create(
                const otio::Gap* gap,
                const ItemData& itemData,
                const std::shared_ptr<system::Context>& context)
            {
                auto out = std::shared_ptr<GapItem>(new GapItem);
                out->_init(gap, itemData, context);
                return out;
            }

            void GapItem::preLayout()
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

            void GapItem::render(
                const std::shared_ptr<timeline::IRender>& render,
                const math::BBox2i& viewport,
                float devicePixelRatio)
            {
                BaseItem::render(render, viewport, devicePixelRatio);

                const math::BBox2i g(
                    _geometry.min.x - viewport.min.x,
                    _geometry.min.y - viewport.min.y,
                    _geometry.w(),
                    _geometry.h());
                const math::BBox2i v(
                    0, 0, viewport.w(), viewport.h());
                if (g.intersects(v))
                {
                    render->drawRect(
                        g * devicePixelRatio,
                        imaging::Color4f(.35, .35, .45));
                    render->drawRect(
                        g.margin(-_itemData.border) * devicePixelRatio,
                        imaging::Color4f(.25, .25, .35));

                    auto fontInfo = _itemData.fontInfo;
                    fontInfo.size *= devicePixelRatio;
                    render->drawText(
                        _itemData.fontSystem->getGlyphs(_label, fontInfo),
                        math::Vector2i(
                            g.min.x +
                            _itemData.border +
                            _itemData.margin,
                            g.min.y +
                            _itemData.border +
                            _itemData.margin +
                            _itemData.fontMetrics.ascender) * devicePixelRatio,
                        imaging::Color4f(.9F, .9F, .9F));
                    render->drawText(
                        _itemData.fontSystem->getGlyphs(_startLabel, fontInfo),
                        math::Vector2i(
                            g.min.x +
                            _itemData.border +
                            _itemData.margin,
                            g.min.y +
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
                            g.max.x -
                            _itemData.border -
                            _itemData.margin -
                            textSize.x,
                            g.min.y +
                            _itemData.border +
                            _itemData.margin +
                            _itemData.fontMetrics.ascender) * devicePixelRatio,
                        imaging::Color4f(.9F, .9F, .9F));
                    textSize = _itemData.fontSystem->measure(_endLabel, _itemData.fontInfo);
                    render->drawText(
                        _itemData.fontSystem->getGlyphs(_endLabel, fontInfo),
                        math::Vector2i(
                            g.max.x -
                            _itemData.border -
                            _itemData.margin -
                            textSize.x,
                            g.min.y +
                            _itemData.border +
                            _itemData.margin +
                            _itemData.fontMetrics.lineHeight +
                            _itemData.spacing +
                            _itemData.fontMetrics.ascender) * devicePixelRatio,
                        imaging::Color4f(.9F, .9F, .9F));
                }
            }

            std::string GapItem::_nameLabel(const std::string& name)
            {
                return !name.empty() ?
                    name :
                    std::string("Gap");
            }
        }
    }
}
