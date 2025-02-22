// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlTimelineUI/IBasicItem.h>

#include <dtk/ui/DrawUtil.h>
#include <dtk/core/RenderUtil.h>

namespace tl
{
    namespace timelineui
    {
        struct IBasicItem::Private
        {
            std::string label;
            std::string durationLabel;
            dtk::Color4F color;
            std::vector<Marker> markers;

            struct SizeData
            {
                bool init = true;
                float displayScale = 0.F;
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
            const std::shared_ptr<dtk::Context>& context,
            const std::string& label,
            const dtk::Color4F& color,
            const std::string& objectName,
            const OTIO_NS::SerializableObject::Retainer<OTIO_NS::Item>& item,
            double scale,
            const ItemOptions& options,
            const DisplayOptions& displayOptions,
            const std::shared_ptr<ItemData>& itemData,
            const std::shared_ptr<dtk::IWidget>& parent)
        {
            OTIO_NS::TimeRange timeRange = time::invalidTimeRange;
            const auto timeRangeOpt = item->trimmed_range_in_parent();
            if (timeRangeOpt.has_value())
            {
                timeRange = timeRangeOpt.value();
            }
            const OTIO_NS::TimeRange trimmedRange = item->trimmed_range();
            IItem::_init(
                context,
                objectName,
                timeRange,
                trimmedRange,
                scale,
                options,
                displayOptions,
                itemData,
                parent);
            DTK_P();

            p.label = label;
            p.color = color;
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

        void IBasicItem::sizeHintEvent(const dtk::SizeHintEvent& event)
        {
            IItem::sizeHintEvent(event);
            DTK_P();

            const bool displayScaleChanged = event.displayScale != p.size.displayScale;
            if (p.size.init || displayScaleChanged)
            {
                p.size.margin = event.style->getSizeRole(dtk::SizeRole::MarginInside, event.displayScale);
                p.size.border = event.style->getSizeRole(dtk::SizeRole::Border, event.displayScale);
            }
            if (p.size.init || displayScaleChanged || p.size.textInit)
            {
                p.size.fontInfo = dtk::FontInfo(
                    _displayOptions.regularFont,
                    _displayOptions.fontSize * event.displayScale);
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
            p.size.init = false;
            p.size.displayScale = event.displayScale;
            p.size.textInit = false;

            dtk::Size2I sizeHint;
            sizeHint.w = _timeRange.duration().rescaled_to(1.0).value() * _scale;
            if (_displayOptions.clipInfo)
            {
                sizeHint.h +=
                    p.size.fontMetrics.lineHeight +
                    p.size.margin * 2;
            }
            sizeHint.h += p.size.border * 4;
            _setSizeHint(sizeHint);
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
            const dtk::DrawEvent& event)
        {
            IItem::drawEvent(drawRect, event);
            DTK_P();

            const dtk::Box2I& g = getGeometry();
            dtk::ColorRole colorRole = getSelectRole();
            if (colorRole != dtk::ColorRole::None)
            {
                event.render->drawMesh(
                    dtk::border(g, p.size.border * 2),
                    event.style->getColorRole(colorRole));
            }

            const dtk::Box2I g2 = dtk::margin(g, -(p.size.border * 2));
            event.render->drawRect(
                g2,
                isEnabled() ? p.color : dtk::greyscale(p.color));

            const dtk::ClipRectEnabledState clipRectEnabledState(event.render);
            const dtk::ClipRectState clipRectState(event.render);
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
                            dtk::ColorRole::Text :
                            dtk::ColorRole::TextDisabled));
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
                            dtk::ColorRole::Text :
                            dtk::ColorRole::TextDisabled));
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
            const dtk::Box2I& g = getGeometry();
            return dtk::margin(g, -(_p->size.border * 2));
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
            _setSizeUpdate();
            _setDrawUpdate();
        }
    }
}
