// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
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
                bool sizeInit = true;
                int margin = 0;
                int border = 0;

                bool textInit = true;
                dtk::FontInfo fontInfo = dtk::FontInfo("", 0);
                dtk::FontMetrics fontMetrics;
                dtk::Size2I labelSize;
                dtk::Size2I durationSize;
            };
            SizeData size;

            struct DrawData
            {
                std::vector<std::shared_ptr<dtk::Glyph> > labelGlyphs;
                std::vector<std::shared_ptr<dtk::Glyph> > durationGlyphs;
            };
            DrawData draw;
        };

        void IBasicItem::_init(
            const std::string& label,
            ui::ColorRole colorRole,
            const std::string& objectName,
            const OTIO_NS::SerializableObject::Retainer<OTIO_NS::Item>& item,
            double scale,
            const ItemOptions& options,
            const DisplayOptions& displayOptions,
            const std::shared_ptr<ItemData>& itemData,
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            OTIO_NS::TimeRange timeRange = time::invalidTimeRange;
            const auto timeRangeOpt = item->trimmed_range_in_parent();
            if (timeRangeOpt.has_value())
            {
                timeRange = timeRangeOpt.value();
            }
            const OTIO_NS::TimeRange trimmedRange = item->trimmed_range();
            IItem::_init(
                objectName,
                timeRange,
                trimmedRange,
                scale,
                options,
                displayOptions,
                itemData,
                context,
                parent);
            DTK_P();

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

        void IBasicItem::setDisplayOptions(const DisplayOptions& value)
        {
            const bool changed = value != _displayOptions;
            IItem::setDisplayOptions(value);
            DTK_P();
            if (changed)
            {
                _textUpdate();
            }
        }

        void IBasicItem::sizeHintEvent(const ui::SizeHintEvent& event)
        {
            const bool displayScaleChanged = event.displayScale != _displayScale;
            IItem::sizeHintEvent(event);
            DTK_P();

            if (displayScaleChanged || p.size.sizeInit)
            {
                p.size.margin = event.style->getSizeRole(ui::SizeRole::MarginInside, _displayScale);
                p.size.border = event.style->getSizeRole(ui::SizeRole::Border, _displayScale);
            }
            if (displayScaleChanged || p.size.textInit || p.size.sizeInit)
            {
                p.size.fontInfo = dtk::FontInfo(
                    _displayOptions.regularFont,
                    _displayOptions.fontSize * _displayScale);
                p.size.fontMetrics = event.fontSystem->getMetrics(p.size.fontInfo);
                p.size.labelSize = _displayOptions.clipInfo ?
                    event.fontSystem->getSize(p.label, p.size.fontInfo) :
                    dtk::Size2I();
                p.size.durationSize = _displayOptions.clipInfo ?
                    event.fontSystem->getSize(p.durationLabel, p.size.fontInfo) :
                    dtk::Size2I();
                p.draw.labelGlyphs.clear();
                p.draw.durationGlyphs.clear();
            }
            p.size.sizeInit = false;
            p.size.textInit = false;

            _sizeHint.w = _timeRange.duration().rescaled_to(1.0).value() * _scale;
            _sizeHint.h = 0;
            if (_displayOptions.clipInfo)
            {
                _sizeHint.h +=
                    p.size.fontMetrics.lineHeight +
                    p.size.margin * 2;
            }
            _sizeHint.h += p.size.border * 4;
        }

        void IBasicItem::clipEvent(const dtk::Box2I& clipRect, bool clipped)
        {
            IItem::clipEvent(clipRect, clipped);
            DTK_P();
            if (clipped)
            {
                p.draw.labelGlyphs.clear();
                p.draw.durationGlyphs.clear();
            }
        }

        void IBasicItem::drawEvent(
            const dtk::Box2I& drawRect,
            const ui::DrawEvent& event)
        {
            IItem::drawEvent(drawRect, event);
            DTK_P();

            const dtk::Box2I& g = _geometry;
            ui::ColorRole colorRole = getSelectRole();
            if (colorRole != ui::ColorRole::None)
            {
                event.render->drawMesh(
                    ui::border(g, p.size.border * 2),
                    event.style->getColorRole(colorRole));
            }

            const dtk::Box2I g2 = dtk::margin(g, -(p.size.border * 2));
            event.render->drawRect(
                g2,
                isEnabled() ?
                    event.style->getColorRole(p.colorRole) :
                    dtk::greyscale(event.style->getColorRole(p.colorRole)));

            const timeline::ClipRectEnabledState clipRectEnabledState(event.render);
            const timeline::ClipRectState clipRectState(event.render);
            event.render->setClipRectEnabled(true);
            event.render->setClipRect(dtk::intersect(g2, drawRect));

            if (_displayOptions.clipInfo)
            {
                const dtk::Box2I labelGeometry(
                    g2.min.x + p.size.margin,
                    g2.min.y + p.size.margin,
                    p.size.labelSize.w,
                    p.size.fontMetrics.lineHeight);
                const bool enabled = isEnabled();
                if (dtk::intersects(drawRect, labelGeometry))
                {
                    if (!p.label.empty() && p.draw.labelGlyphs.empty())
                    {
                        p.draw.labelGlyphs = event.fontSystem->getGlyphs(p.label, p.size.fontInfo);
                    }
                    event.render->drawText(
                        p.draw.labelGlyphs,
                        p.size.fontMetrics,
                        labelGeometry.min,
                        event.style->getColorRole(
                            enabled ?
                            ui::ColorRole::Text :
                            ui::ColorRole::TextDisabled));
                }

                const dtk::Box2I durationGeometry(
                    g2.max.x -
                    p.size.durationSize.w -
                    p.size.margin,
                    g2.min.y + p.size.margin,
                    p.size.durationSize.w,
                    p.size.fontMetrics.lineHeight);
                if (dtk::intersects(drawRect, durationGeometry) &&
                    !dtk::intersects(durationGeometry, labelGeometry))
                {
                    if (!p.durationLabel.empty() && p.draw.durationGlyphs.empty())
                    {
                        p.draw.durationGlyphs = event.fontSystem->getGlyphs(p.durationLabel, p.size.fontInfo);
                    }
                    event.render->drawText(
                        p.draw.durationGlyphs,
                        p.size.fontMetrics,
                        durationGeometry.min,
                        event.style->getColorRole(
                            enabled ?
                            ui::ColorRole::Text :
                            ui::ColorRole::TextDisabled));
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

        dtk::Box2I IBasicItem::_getInsideGeometry() const
        {
            return dtk::margin(_geometry, -(_p->size.border * 2));
        }

        void IBasicItem::_timeUnitsUpdate()
        {
            IItem::_timeUnitsUpdate();
            _textUpdate();
        }

        void IBasicItem::_textUpdate()
        {
            DTK_P();
            p.durationLabel = _getDurationLabel(_timeRange.duration());
            p.size.textInit = true;
            _updates |= ui::Update::Size;
            _updates |= ui::Update::Draw;
        }
    }
}
