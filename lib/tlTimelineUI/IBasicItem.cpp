// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <tlTimelineUI/IBasicItem.h>

#include <ftk/UI/DrawUtil.h>
#include <ftk/Core/RenderUtil.h>

namespace tl
{
    namespace timelineui
    {
        struct IBasicItem::Private
        {
            std::string label;
            std::string durationLabel;
            ftk::ColorRole colorRole = ftk::ColorRole::None;
            std::vector<Marker> markers;

            struct SizeData
            {
                std::optional<float> displayScale;
                int margin = 0;
                int border = 0;
                ftk::FontInfo fontInfo = ftk::FontInfo("", 0);
                ftk::FontMetrics fontMetrics;
                ftk::Size2I labelSize;
                ftk::Size2I durationSize;
            };
            SizeData size;

            struct DrawData
            {
                ftk::Box2I g;
                ftk::Box2I g2;
                ftk::Box2I labelGeometry;
                ftk::Box2I durationGeometry;
                ftk::TriMesh2F border;
                std::vector<std::shared_ptr<ftk::Glyph> > labelGlyphs;
                std::vector<std::shared_ptr<ftk::Glyph> > durationGlyphs;
            };
            std::optional<DrawData> draw;
        };

        void IBasicItem::_init(
            const std::shared_ptr<ftk::Context>& context,
            const std::string& label,
            ftk::ColorRole colorRole,
            const std::string& objectName,
            const OTIO_NS::SerializableObject::Retainer<OTIO_NS::Item>& item,
            double scale,
            const ItemOptions& options,
            const DisplayOptions& displayOptions,
            const std::shared_ptr<ItemData>& itemData,
            const std::shared_ptr<ftk::IWidget>& parent)
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
            FTK_P();

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
            FTK_P();
            if (changed)
            {
                p.draw.reset();
            }
        }

        void IBasicItem::setDisplayOptions(const DisplayOptions& value)
        {
            const bool changed = value != _displayOptions;
            IItem::setDisplayOptions(value);
            FTK_P();
            if (changed)
            {
                _textUpdate();
            }
        }

        void IBasicItem::setGeometry(const ftk::Box2I& value)
        {
            bool changed = value != getGeometry();
            IItem::setGeometry(value);
            FTK_P();
            if (changed)
            {
                p.draw.reset();
            }
        }

        void IBasicItem::sizeHintEvent(const ftk::SizeHintEvent& event)
        {
            IItem::sizeHintEvent(event);
            FTK_P();

            if (!p.size.displayScale.has_value() ||
                (p.size.displayScale.has_value() && p.size.displayScale.value() != event.displayScale))
            {
                p.size.displayScale = event.displayScale;
                p.size.margin = event.style->getSizeRole(ftk::SizeRole::MarginInside, event.displayScale);
                p.size.border = event.style->getSizeRole(ftk::SizeRole::Border, event.displayScale);
                p.size.fontInfo = ftk::FontInfo(
                    _displayOptions.regularFont,
                    _displayOptions.fontSize * event.displayScale);
                p.size.fontMetrics = event.fontSystem->getMetrics(p.size.fontInfo);
                p.size.labelSize = !_displayOptions.minimize ?
                    event.fontSystem->getSize(p.label, p.size.fontInfo) :
                    ftk::Size2I();
                p.size.durationSize = !_displayOptions.minimize ?
                    event.fontSystem->getSize(p.durationLabel, p.size.fontInfo) :
                    ftk::Size2I();
                p.draw.reset();
            }

            ftk::Size2I sizeHint;
            sizeHint.w = _timeRange.duration().rescaled_to(1.0).value() * _scale;
            if (!_displayOptions.minimize)
            {
                sizeHint.h +=
                    p.size.fontMetrics.lineHeight +
                    p.size.margin * 2;
            }
            sizeHint.h += p.size.border * 4;
            _setSizeHint(sizeHint);
        }

        void IBasicItem::clipEvent(const ftk::Box2I& clipRect, bool clipped)
        {
            IItem::clipEvent(clipRect, clipped);
            FTK_P();
            if (clipped)
            {
                p.draw.reset();
            }
        }

        void IBasicItem::drawEvent(
            const ftk::Box2I& drawRect,
            const ftk::DrawEvent& event)
        {
            IItem::drawEvent(drawRect, event);
            FTK_P();

            if (!p.draw.has_value())
            {
                p.draw = Private::DrawData();
                p.draw->g = getGeometry();
                p.draw->g2 = ftk::margin(p.draw->g, -(p.size.border * 2));
                p.draw->labelGeometry = ftk::Box2I(
                    p.draw->g2.min.x + p.size.margin,
                    p.draw->g2.min.y + p.size.margin,
                    p.size.labelSize.w,
                    p.size.fontMetrics.lineHeight);
                p.draw->durationGeometry = ftk::Box2I(
                    p.draw->g2.max.x -
                    p.size.durationSize.w -
                    p.size.margin,
                    p.draw->g2.min.y + p.size.margin,
                    p.size.durationSize.w,
                    p.size.fontMetrics.lineHeight);
                p.draw->border = ftk::border(p.draw->g, p.size.border * 2);
            }

            // Draw the selection border.
            ftk::ColorRole colorRole = getSelectRole();
            if (colorRole != ftk::ColorRole::None)
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
                    ftk::greyscale(event.style->getColorRole(p.colorRole)));

            // Draw the labels.
            if (!_displayOptions.minimize)
            {
                std::unique_ptr<ftk::ClipRectEnabledState> clipRectEnabledState;
                std::unique_ptr<ftk::ClipRectState> clipRectState;
                if (!ftk::contains(p.draw->g2, p.draw->labelGeometry) ||
                    !ftk::contains(p.draw->g2, p.draw->durationGeometry))
                {
                    clipRectEnabledState.reset(new ftk::ClipRectEnabledState(event.render));
                    clipRectState.reset(new ftk::ClipRectState(event.render));
                    event.render->setClipRectEnabled(true);
                    event.render->setClipRect(ftk::intersect(p.draw->g2, drawRect));
                }

                const bool enabled = isEnabled();
                if (ftk::intersects(drawRect, p.draw->labelGeometry))
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
                            ftk::ColorRole::Text :
                            ftk::ColorRole::TextDisabled));
                }

                if (ftk::intersects(drawRect, p.draw->durationGeometry) &&
                    !ftk::intersects(p.draw->durationGeometry, p.draw->labelGeometry))
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
                            ftk::ColorRole::Text :
                            ftk::ColorRole::TextDisabled));
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

        ftk::Box2I IBasicItem::_getInsideGeometry() const
        {
            const ftk::Box2I& g = getGeometry();
            return ftk::margin(g, -(_p->size.border * 2));
        }

        void IBasicItem::_timeUnitsUpdate()
        {
            IItem::_timeUnitsUpdate();
            _textUpdate();
        }

        void IBasicItem::_textUpdate()
        {
            FTK_P();
            p.durationLabel = _getDurationLabel(_timeRange.duration());
            p.size.displayScale.reset();
            setSizeUpdate();
            setDrawUpdate();
        }
    }
}
