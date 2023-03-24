// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include "ClipItem.h"

#include <tlUI/DrawUtil.h>

#include <QPainter>

namespace tl
{
    namespace examples
    {
        namespace timeline_qtwidget
        {
            void  ClipItem::_init(
                const otio::Clip* clip,
                const std::shared_ptr<system::Context>& context,
                const std::shared_ptr<IWidget>& parent)
            {
                IItem::_init("ClipItem", context, parent);

                auto rangeOpt = clip->trimmed_range_in_parent();
                if (rangeOpt.has_value())
                {
                    _timeRange = rangeOpt.value();
                }

                _label = _nameLabel(clip->name());
                _durationLabel = IItem::_durationLabel(_timeRange.duration());
                _startLabel = _timeLabel(_timeRange.start_time());
                _endLabel = _timeLabel(_timeRange.end_time_inclusive());
            }

            ClipItem::~ClipItem()
            {}

            std::shared_ptr<ClipItem>  ClipItem::create(
                const otio::Clip* clip,
                const std::shared_ptr<system::Context>& context,
                const std::shared_ptr<IWidget>& parent)
            {
                auto out = std::shared_ptr<ClipItem>(new ClipItem);
                out->_init(clip, context, parent);
                return out;
            }

            void ClipItem::sizeEvent(const ui::SizeEvent& event)
            {
                IItem::sizeEvent(event);

                const int m = event.style->getSizeRole(ui::SizeRole::MarginSmall) * event.contentScale;
                const int s = event.style->getSizeRole(ui::SizeRole::SpacingSmall) * event.contentScale;
                auto fontInfo = _fontInfo;
                fontInfo.size *= event.contentScale;
                const auto fontMetrics = event.fontSystem->getMetrics(fontInfo);

                _sizeHint = math::Vector2i(
                    _timeRange.duration().rescaled_to(1.0).value() * _scale,
                    m +
                    fontMetrics.lineHeight +
                    s +
                    fontMetrics.lineHeight +
                    m);
            }

            void ClipItem::drawEvent(const ui::DrawEvent& event)
            {
                IItem::drawEvent(event);

                const int m = event.style->getSizeRole(ui::SizeRole::MarginSmall) * event.contentScale;
                const int s = event.style->getSizeRole(ui::SizeRole::SpacingSmall) * event.contentScale;
                const int b = event.style->getSizeRole(ui::SizeRole::Border) * event.contentScale;
                auto fontInfo = _fontInfo;
                fontInfo.size *= event.contentScale;
                const auto fontMetrics = event.fontSystem->getMetrics(fontInfo);

                math::BBox2i g = _geometry;
                g.min = g.min - _viewport.min;
                g.max = g.max - _viewport.min;

                event.render->drawMesh(
                    ui::border(g, b, m / 2),
                    event.style->getColorRole(ui::ColorRole::Border));

                event.render->drawRect(
                    g.margin(-b),
                    event.style->getColorRole(ui::ColorRole::Green));

                event.render->drawText(
                    event.fontSystem->getGlyphs(_label, fontInfo),
                    math::Vector2i(
                        g.min.x +
                        m,
                        g.min.y +
                        m +
                        fontMetrics.ascender),
                    event.style->getColorRole(ui::ColorRole::Text));
                event.render->drawText(
                    event.fontSystem->getGlyphs(_startLabel, fontInfo),
                    math::Vector2i(
                        g.min.x +
                        m,
                        g.min.y +
                        m +
                        fontMetrics.lineHeight +
                        s +
                        fontMetrics.ascender),
                    event.style->getColorRole(ui::ColorRole::Text));

                math::Vector2i textSize = event.fontSystem->measure(_durationLabel, fontInfo);
                event.render->drawText(
                    event.fontSystem->getGlyphs(_durationLabel, fontInfo),
                    math::Vector2i(
                        g.max.x -
                        m -
                        textSize.x,
                        g.min.y +
                        m +
                        fontMetrics.ascender),
                    event.style->getColorRole(ui::ColorRole::Text));
                textSize = event.fontSystem->measure(_endLabel, fontInfo);
                event.render->drawText(
                    event.fontSystem->getGlyphs(_endLabel, fontInfo),
                    math::Vector2i(
                        g.max.x -
                        m -
                        textSize.x,
                        g.min.y +
                        m +
                        fontMetrics.lineHeight +
                        s +
                        fontMetrics.ascender),
                    event.style->getColorRole(ui::ColorRole::Text));
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
