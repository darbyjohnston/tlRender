// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlTimelineUI/IBasicItem.h>

#include <tlUI/DrawUtil.h>

#include <tlTimeline/RenderUtil.h>

namespace tl
{
    namespace timelineui
    {
        struct IBasicItem::Private
        {
            std::string label;
            std::string durationLabel;
            ui::ColorRole colorRole = ui::ColorRole::VideoClip;
            std::vector<Marker> markers;

            struct SizeData
            {
                int margin = 0;
                int border = 0;
                image::FontInfo fontInfo = image::FontInfo("", 0);
                image::FontMetrics fontMetrics;
                bool textUpdate = true;
                math::Size2i labelSize;
                math::Size2i durationSize;
                std::vector<math::Size2i> markerSizes;
            };
            SizeData size;

            struct DrawData
            {
                std::vector<std::shared_ptr<image::Glyph> > labelGlyphs;
                std::vector<std::shared_ptr<image::Glyph> > durationGlyphs;
                std::vector<std::vector<std::shared_ptr<image::Glyph> > > markerGlyphs;
            };
            DrawData draw;
        };

        void IBasicItem::_init(
            const std::string& label,
            ui::ColorRole colorRole,
            const std::string& objectName,
            const otio::SerializableObject::Retainer<otio::Item>& item,
            const ItemData& itemData,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            const auto timeRangeOpt = item->trimmed_range_in_parent();
            IItem::_init(
                objectName,
                item.value,
                timeRangeOpt.has_value() ? timeRangeOpt.value() : time::invalidTimeRange,
                itemData,
                context,
                parent);
            TLRENDER_P();

            p.label = label;
            p.colorRole = colorRole;
            p.markers = getMarkers(item.value);

            _textUpdate();
        }

        IBasicItem::IBasicItem() :
            _p(new Private)
        {}

        IBasicItem::~IBasicItem()
        {}

        void IBasicItem::sizeHintEvent(const ui::SizeHintEvent& event)
        {
            IItem::sizeHintEvent(event);
            TLRENDER_P();

            p.size.margin = event.style->getSizeRole(ui::SizeRole::MarginInside, event.displayScale);
            p.size.border = event.style->getSizeRole(ui::SizeRole::Border, event.displayScale);

            auto fontInfo = image::FontInfo(
                _options.regularFont,
                _options.fontSize * event.displayScale);
            if (fontInfo != p.size.fontInfo || p.size.textUpdate)
            {
                p.size.fontInfo = fontInfo;
                p.size.fontMetrics = event.fontSystem->getMetrics(fontInfo);
                p.size.labelSize = event.fontSystem->getSize(p.label, fontInfo);
                p.size.durationSize = event.fontSystem->getSize(p.durationLabel, fontInfo);
                p.size.markerSizes.clear();
                for (const auto& marker : p.markers)
                {
                    p.size.markerSizes.push_back(
                        event.fontSystem->getSize(marker.name, fontInfo));
                }
                p.draw.labelGlyphs.clear();
                p.draw.durationGlyphs.clear();
                p.draw.markerGlyphs.clear();
            }
            p.size.textUpdate = false;

            _sizeHint = math::Size2i(
                _timeRange.duration().rescaled_to(1.0).value() * _scale,
                p.size.fontMetrics.lineHeight +
                p.size.margin * 2 +
                p.size.border * 4);
            if (_options.showMarkers)
            {
                _sizeHint.h += p.markers.size() *
                    (p.size.fontMetrics.lineHeight +
                    p.size.margin * 2 +
                    p.size.border * 2);
            }
        }

        void IBasicItem::clipEvent(
            const math::Box2i& clipRect,
            bool clipped,
            const ui::ClipEvent& event)
        {
            IItem::clipEvent(clipRect, clipped, event);
            TLRENDER_P();
            if (clipped)
            {
                p.draw.labelGlyphs.clear();
                p.draw.durationGlyphs.clear();
                p.draw.markerGlyphs.clear();
            }
        }

        void IBasicItem::drawEvent(
            const math::Box2i& drawRect,
            const ui::DrawEvent& event)
        {
            IItem::drawEvent(drawRect, event);
            TLRENDER_P();

            const math::Box2i& g = _geometry;
            ui::ColorRole colorRole = getSelectRole();
            if (colorRole != ui::ColorRole::None)
            {
                event.render->drawMesh(
                    ui::border(g, p.size.border * 2),
                    math::Vector2i(),
                    event.style->getColorRole(colorRole));
            }

            const math::Box2i g2 = g.margin(-(p.size.border * 2));
            event.render->drawRect(
                g2,
                event.style->getColorRole(p.colorRole));

            const timeline::ClipRectEnabledState clipRectEnabledState(event.render);
            const timeline::ClipRectState clipRectState(event.render);
            event.render->setClipRectEnabled(true);
            event.render->setClipRect(g2.intersect(drawRect));

            math::Box2i labelGeometry(
                g2.min.x + p.size.margin,
                g2.min.y + p.size.margin,
                p.size.labelSize.w,
                p.size.fontMetrics.lineHeight);
            if (drawRect.intersects(labelGeometry))
            {
                if (!p.label.empty() && p.draw.labelGlyphs.empty())
                {
                    p.draw.labelGlyphs = event.fontSystem->getGlyphs(p.label, p.size.fontInfo);
                }
                event.render->drawText(
                    p.draw.labelGlyphs,
                    math::Vector2i(
                        labelGeometry.min.x,
                        labelGeometry.min.y +
                        p.size.fontMetrics.ascender),
                    event.style->getColorRole(ui::ColorRole::Text));
            }

            const math::Box2i durationGeometry(
                g2.max.x -
                p.size.durationSize.w -
                p.size.margin,
                g2.min.y + p.size.margin,
                p.size.durationSize.w,
                p.size.fontMetrics.lineHeight);
            if (drawRect.intersects(durationGeometry) &&
                !durationGeometry.intersects(labelGeometry))
            {
                if (!p.durationLabel.empty() && p.draw.durationGlyphs.empty())
                {
                    p.draw.durationGlyphs = event.fontSystem->getGlyphs(p.durationLabel, p.size.fontInfo);
                }
                event.render->drawText(
                    p.draw.durationGlyphs,
                    math::Vector2i(
                        durationGeometry.min.x,
                        durationGeometry.min.y +
                        p.size.fontMetrics.ascender),
                    event.style->getColorRole(ui::ColorRole::Text));
            }

            if (_options.showMarkers)
            {
                if (p.draw.markerGlyphs.empty())
                {
                    p.draw.markerGlyphs.resize(p.markers.size());
                }
                float y = g2.max.y + 1 -
                    (p.size.fontMetrics.lineHeight +
                    p.size.margin * 2 +
                    p.size.border * 2) *
                    p.markers.size();
                for (size_t i = 0; i < p.markers.size(); ++i)
                {
                    const int x0 =
                        _geometry.min.x +
                        p.markers[i].range.start_time().rescaled_to(1.0).value() * _scale;
                    const int x1 =
                        _geometry.min.x +
                        p.markers[i].range.end_time_exclusive().rescaled_to(1.0).value() * _scale - 1;
                    math::Box2i g2;
                    g2.min.x = std::max(x0, g2.min.x);
                    g2.min.y = y;
                    g2.max.x = std::min(x1, g2.max.x);
                    g2.max.y = y + p.size.border * 2 - 1;
                    event.render->drawRect(g2, p.markers[i].color);

                    y += p.size.border * 2;

                    labelGeometry = math::Box2i(
                        g2.min.x + p.size.margin,
                        y + p.size.margin,
                        p.size.markerSizes[i].w,
                        p.size.fontMetrics.lineHeight);
                    if (drawRect.intersects(labelGeometry))
                    {
                        if (!p.markers[i].name.empty() && p.draw.markerGlyphs[i].empty())
                        {
                            p.draw.markerGlyphs[i] = event.fontSystem->getGlyphs(p.markers[i].name, p.size.fontInfo);
                        }
                        event.render->drawText(
                            p.draw.markerGlyphs[i],
                            math::Vector2i(
                                labelGeometry.min.x,
                                labelGeometry.min.y +
                                p.size.fontMetrics.ascender),
                            event.style->getColorRole(ui::ColorRole::Text));
                    }

                    y += p.size.fontMetrics.lineHeight +
                        p.size.margin * 2;
                }
            }
        }

        int IBasicItem::_getMargin() const
        {
            return _p->size.margin;
        }

        int IBasicItem::_getLineHeight() const
        {
            return _p->size.fontMetrics.lineHeight;
        }

        math::Box2i IBasicItem::_getInsideGeometry() const
        {
            return _geometry.margin(-(_p->size.border * 2));
        }

        void IBasicItem::_timeUnitsUpdate()
        {
            IItem::_timeUnitsUpdate();
            _textUpdate();
        }

        void IBasicItem::_textUpdate()
        {
            TLRENDER_P();
            p.durationLabel = _getDurationLabel(_timeRange.duration());
            p.size.textUpdate = true;
            p.draw.durationGlyphs.clear();
            _updates |= ui::Update::Size;
            _updates |= ui::Update::Draw;
        }
    }
}
