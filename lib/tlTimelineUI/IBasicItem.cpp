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
            dtk::ColorRole colorRole = dtk::ColorRole::None;
            std::vector<Marker> markers;

            struct SizeData
            {
                std::optional<float> displayScale;
                int margin = 0;
                int border = 0;
                dtk::FontInfo fontInfo = dtk::FontInfo("", 0);
                dtk::FontMetrics fontMetrics;
                dtk::Size2I labelSize;
                dtk::Size2I durationSize;
            };
            SizeData size;

            struct DrawData
            {
                dtk::Box2I g;
                dtk::Box2I g2;
                dtk::Box2I labelGeometry;
                dtk::Box2I durationGeometry;
                dtk::TriMesh2F border;
                std::vector<std::shared_ptr<dtk::Glyph> > labelGlyphs;
                std::vector<std::shared_ptr<dtk::Glyph> > durationGlyphs;
            };
            std::optional<DrawData> draw;
        };

        void IBasicItem::_init(
            const std::shared_ptr<dtk::Context>& context,
            const std::string& label,
            dtk::ColorRole colorRole,
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
            p.colorRole = colorRole;
            p.markers = getMarkers(item.value);

            _textUpdate();
        }

        IBasicItem::IBasicItem() :
            _p(new Private)
        {}

        IBasicItem::~IBasicItem()
        {}

        void IBasicItem::setScale(double value)
        {
            const bool changed = value != _scale;
            IItem::setScale(value);
            DTK_P();
            if (changed)
            {
                p.draw.reset();
            }
        }

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

        void IBasicItem::setGeometry(const dtk::Box2I& value)
        {
            bool changed = value != getGeometry();
            IItem::setGeometry(value);
            DTK_P();
            if (changed)
            {
                p.draw.reset();
            }
        }

        void IBasicItem::sizeHintEvent(const dtk::SizeHintEvent& event)
        {
            IItem::sizeHintEvent(event);
            DTK_P();

            if (!p.size.displayScale.has_value() ||
                (p.size.displayScale.has_value() && p.size.displayScale.value() != event.displayScale))
            {
                p.size.displayScale = event.displayScale;
                p.size.margin = event.style->getSizeRole(dtk::SizeRole::MarginInside, event.displayScale);
                p.size.border = event.style->getSizeRole(dtk::SizeRole::Border, event.displayScale);
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
                p.draw.reset();
            }

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
                p.draw.reset();
            }
        }

        void IBasicItem::drawEvent(
            const dtk::Box2I& drawRect,
            const dtk::DrawEvent& event)
        {
            IItem::drawEvent(drawRect, event);
            DTK_P();

            if (!p.draw.has_value())
            {
                p.draw = Private::DrawData();
                p.draw->g = getGeometry();
                p.draw->g2 = dtk::margin(p.draw->g, -(p.size.border * 2));
                p.draw->labelGeometry = dtk::Box2I(
                    p.draw->g2.min.x + p.size.margin,
                    p.draw->g2.min.y + p.size.margin,
                    p.size.labelSize.w,
                    p.size.fontMetrics.lineHeight);
                p.draw->durationGeometry = dtk::Box2I(
                    p.draw->g2.max.x -
                    p.size.durationSize.w -
                    p.size.margin,
                    p.draw->g2.min.y + p.size.margin,
                    p.size.durationSize.w,
                    p.size.fontMetrics.lineHeight);
                p.draw->border = dtk::border(p.draw->g, p.size.border * 2);
            }

            // Draw the selection border.
            dtk::ColorRole colorRole = getSelectRole();
            if (colorRole != dtk::ColorRole::None)
            {
                event.render->drawMesh(
                    p.draw->border,
                    event.style->getColorRole(colorRole));
            }

            // Draw the background.
            event.render->drawRect(
                p.draw->g2,
                isEnabled() ?
                    event.style->getColorRole(p.colorRole) :
                    dtk::greyscale(event.style->getColorRole(p.colorRole)));

            // Draw the labels.
            if (_displayOptions.clipInfo)
            {
                const dtk::ClipRectEnabledState clipRectEnabledState(event.render);
                const dtk::ClipRectState clipRectState(event.render);
                event.render->setClipRectEnabled(true);
                event.render->setClipRect(dtk::intersect(p.draw->g2, drawRect));

                const bool enabled = isEnabled();
                if (dtk::intersects(drawRect, p.draw->labelGeometry))
                {
                    if (!p.label.empty() && p.draw->labelGlyphs.empty())
                    {
                        p.draw->labelGlyphs = event.fontSystem->getGlyphs(p.label, p.size.fontInfo);
                    }
                    event.render->drawText(
                        p.draw->labelGlyphs,
                        p.size.fontMetrics,
                        p.draw->labelGeometry.min,
                        event.style->getColorRole(
                            enabled ?
                            dtk::ColorRole::Text :
                            dtk::ColorRole::TextDisabled));
                }

                if (dtk::intersects(drawRect, p.draw->durationGeometry) &&
                    !dtk::intersects(p.draw->durationGeometry, p.draw->labelGeometry))
                {
                    if (!p.durationLabel.empty() && p.draw->durationGlyphs.empty())
                    {
                        p.draw->durationGlyphs = event.fontSystem->getGlyphs(p.durationLabel, p.size.fontInfo);
                    }
                    event.render->drawText(
                        p.draw->durationGlyphs,
                        p.size.fontMetrics,
                        p.draw->durationGeometry.min,
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
            p.size.displayScale.reset();
            _setSizeUpdate();
            _setDrawUpdate();
        }
    }
}
