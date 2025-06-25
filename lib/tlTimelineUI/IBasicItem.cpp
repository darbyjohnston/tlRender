// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlTimelineUI/IBasicItem.h>

#include <feather-tk/ui/DrawUtil.h>
#include <feather-tk/core/RenderUtil.h>

namespace tl
{
    namespace timelineui
    {
        struct IBasicItem::Private
        {
            std::string label;
            std::string durationLabel;
            feather_tk::ColorRole colorRole = feather_tk::ColorRole::None;
            std::vector<Marker> markers;

            struct SizeData
            {
                std::optional<float> displayScale;
                int margin = 0;
                int border = 0;
                feather_tk::FontInfo fontInfo = feather_tk::FontInfo("", 0);
                feather_tk::FontMetrics fontMetrics;
                feather_tk::Size2I labelSize;
                feather_tk::Size2I durationSize;
            };
            SizeData size;

            struct DrawData
            {
                feather_tk::Box2I g;
                feather_tk::Box2I g2;
                feather_tk::Box2I labelGeometry;
                feather_tk::Box2I durationGeometry;
                feather_tk::TriMesh2F border;
                std::vector<std::shared_ptr<feather_tk::Glyph> > labelGlyphs;
                std::vector<std::shared_ptr<feather_tk::Glyph> > durationGlyphs;
            };
            std::optional<DrawData> draw;
        };

        void IBasicItem::_init(
            const std::shared_ptr<feather_tk::Context>& context,
            const std::string& label,
            feather_tk::ColorRole colorRole,
            const std::string& objectName,
            const OTIO_NS::SerializableObject::Retainer<OTIO_NS::Item>& item,
            double scale,
            const ItemOptions& options,
            const DisplayOptions& displayOptions,
            const std::shared_ptr<ItemData>& itemData,
            const std::shared_ptr<feather_tk::IWidget>& parent)
        {
            OTIO_NS::TimeRange timeRange = time::invalidTimeRange;
            const auto timeRangeOpt = item->trimmed_range_in_parent();
            if (timeRangeOpt.has_value())
            {
                timeRange = timeRangeOpt.value();
            }
            const OTIO_NS::TimeRange availableRange = item->available_range();
            const OTIO_NS::TimeRange trimmedRange = item->trimmed_range();
            IItem::_init(
                context,
                objectName,
                timeRange,
                availableRange,
                trimmedRange,
                scale,
                options,
                displayOptions,
                itemData,
                parent);
            FEATHER_TK_P();

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
            FEATHER_TK_P();
            if (changed)
            {
                p.draw.reset();
            }
        }

        void IBasicItem::setDisplayOptions(const DisplayOptions& value)
        {
            const bool changed = value != _displayOptions;
            IItem::setDisplayOptions(value);
            FEATHER_TK_P();
            if (changed)
            {
                _textUpdate();
            }
        }

        void IBasicItem::setGeometry(const feather_tk::Box2I& value)
        {
            bool changed = value != getGeometry();
            IItem::setGeometry(value);
            FEATHER_TK_P();
            if (changed)
            {
                p.draw.reset();
            }
        }

        void IBasicItem::sizeHintEvent(const feather_tk::SizeHintEvent& event)
        {
            IItem::sizeHintEvent(event);
            FEATHER_TK_P();

            if (!p.size.displayScale.has_value() ||
                (p.size.displayScale.has_value() && p.size.displayScale.value() != event.displayScale))
            {
                p.size.displayScale = event.displayScale;
                p.size.margin = event.style->getSizeRole(feather_tk::SizeRole::MarginInside, event.displayScale);
                p.size.border = event.style->getSizeRole(feather_tk::SizeRole::Border, event.displayScale);
                p.size.fontInfo = feather_tk::FontInfo(
                    _displayOptions.regularFont,
                    _displayOptions.fontSize * event.displayScale);
                p.size.fontMetrics = event.fontSystem->getMetrics(p.size.fontInfo);
                p.size.labelSize = _displayOptions.clipInfo ?
                    event.fontSystem->getSize(p.label, p.size.fontInfo) :
                    feather_tk::Size2I();
                p.size.durationSize = _displayOptions.clipInfo ?
                    event.fontSystem->getSize(p.durationLabel, p.size.fontInfo) :
                    feather_tk::Size2I();
                p.draw.reset();
            }

            feather_tk::Size2I sizeHint;
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

        void IBasicItem::clipEvent(const feather_tk::Box2I& clipRect, bool clipped)
        {
            IItem::clipEvent(clipRect, clipped);
            FEATHER_TK_P();
            if (clipped)
            {
                p.draw.reset();
            }
        }

        void IBasicItem::drawEvent(
            const feather_tk::Box2I& drawRect,
            const feather_tk::DrawEvent& event)
        {
            IItem::drawEvent(drawRect, event);
            FEATHER_TK_P();

            if (!p.draw.has_value())
            {
                p.draw = Private::DrawData();
                p.draw->g = getGeometry();
                p.draw->g2 = feather_tk::margin(p.draw->g, -(p.size.border * 2));
                p.draw->labelGeometry = feather_tk::Box2I(
                    p.draw->g2.min.x + p.size.margin,
                    p.draw->g2.min.y + p.size.margin,
                    p.size.labelSize.w,
                    p.size.fontMetrics.lineHeight);
                p.draw->durationGeometry = feather_tk::Box2I(
                    p.draw->g2.max.x -
                    p.size.durationSize.w -
                    p.size.margin,
                    p.draw->g2.min.y + p.size.margin,
                    p.size.durationSize.w,
                    p.size.fontMetrics.lineHeight);
                p.draw->border = feather_tk::border(p.draw->g, p.size.border * 2);
            }

            // Draw the selection border.
            feather_tk::ColorRole colorRole = getSelectRole();
            if (colorRole != feather_tk::ColorRole::None)
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
                    feather_tk::greyscale(event.style->getColorRole(p.colorRole)));

            // Draw the labels.
            if (_displayOptions.clipInfo)
            {
                std::unique_ptr<feather_tk::ClipRectEnabledState> clipRectEnabledState;
                std::unique_ptr<feather_tk::ClipRectState> clipRectState;
                if (!feather_tk::contains(p.draw->g2, p.draw->labelGeometry) ||
                    !feather_tk::contains(p.draw->g2, p.draw->durationGeometry))
                {
                    clipRectEnabledState.reset(new feather_tk::ClipRectEnabledState(event.render));
                    clipRectState.reset(new feather_tk::ClipRectState(event.render));
                    event.render->setClipRectEnabled(true);
                    event.render->setClipRect(feather_tk::intersect(p.draw->g2, drawRect));
                }

                const bool enabled = isEnabled();
                if (feather_tk::intersects(drawRect, p.draw->labelGeometry))
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
                            feather_tk::ColorRole::Text :
                            feather_tk::ColorRole::TextDisabled));
                }

                if (feather_tk::intersects(drawRect, p.draw->durationGeometry) &&
                    !feather_tk::intersects(p.draw->durationGeometry, p.draw->labelGeometry))
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
                            feather_tk::ColorRole::Text :
                            feather_tk::ColorRole::TextDisabled));
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

        feather_tk::Box2I IBasicItem::_getInsideGeometry() const
        {
            const feather_tk::Box2I& g = getGeometry();
            return feather_tk::margin(g, -(_p->size.border * 2));
        }

        void IBasicItem::_timeUnitsUpdate()
        {
            IItem::_timeUnitsUpdate();
            _textUpdate();
        }

        void IBasicItem::_textUpdate()
        {
            FEATHER_TK_P();
            p.durationLabel = _getDurationLabel(_timeRange.duration());
            p.size.displayScale.reset();
            _setSizeUpdate();
            _setDrawUpdate();
        }
    }
}
