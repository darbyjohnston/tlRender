// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlTimelineUI/AudioGapItem.h>

#include <tlUI/DrawUtil.h>

namespace tl
{
    namespace timelineui
    {
        struct AudioGapItem::Private
        {
            otime::TimeRange timeRange = time::invalidTimeRange;
            std::string label;
            std::string durationLabel;
            ui::FontRole fontRole = ui::FontRole::Label;

            struct SizeData
            {
                int margin = 0;
                int spacing = 0;
                int border = 0;
                math::Vector2i labelSize;
                math::Vector2i durationSize;
            };
            SizeData size;

            struct DrawData
            {
                std::vector<std::shared_ptr<imaging::Glyph> > labelGlyphs;
                std::vector<std::shared_ptr<imaging::Glyph> > durationGlyphs;
            };
            DrawData draw;
        };

        void AudioGapItem::_init(
            const otio::Gap* gap,
            const ItemData& itemData,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IItem::_init("tl::timelineui::AudioGapItem", itemData, context, parent);
            TLRENDER_P();

            auto rangeOpt = gap->trimmed_range_in_parent();
            if (rangeOpt.has_value())
            {
                p.timeRange = rangeOpt.value();
            }

            p.label = _nameLabel(gap->name());
            _textUpdate();
        }

        AudioGapItem::AudioGapItem() :
            _p(new Private)
        {}

        AudioGapItem::~AudioGapItem()
        {}

        std::shared_ptr<AudioGapItem> AudioGapItem::create(
            const otio::Gap* gap,
            const ItemData& itemData,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<AudioGapItem>(new AudioGapItem);
            out->_init(gap, itemData, context, parent);
            return out;
        }

        void AudioGapItem::setOptions(const ItemOptions& value)
        {
            IItem::setOptions(value);
            if (_updates & ui::Update::Size)
            {
                _textUpdate();
            }
        }

        void AudioGapItem::sizeHintEvent(const ui::SizeHintEvent& event)
        {
            IItem::sizeHintEvent(event);
            TLRENDER_P();

            p.size.margin = event.style->getSizeRole(ui::SizeRole::MarginSmall, event.displayScale);
            p.size.spacing = event.style->getSizeRole(ui::SizeRole::SpacingSmall, event.displayScale);
            p.size.border = event.style->getSizeRole(ui::SizeRole::Border, event.displayScale);

            const auto fontInfo = event.style->getFontRole(p.fontRole, event.displayScale);
            const auto fontMetrics = event.getFontMetrics(p.fontRole);
            p.size.labelSize = event.fontSystem->getSize(p.label, fontInfo);
            p.size.durationSize = event.fontSystem->getSize(p.durationLabel, fontInfo);

            _sizeHint = math::Vector2i(
                p.timeRange.duration().rescaled_to(1.0).value() * _scale,
                p.size.margin +
                fontMetrics.lineHeight +
                p.size.margin);
        }

        void AudioGapItem::clipEvent(
            const math::BBox2i& clipRect,
            bool clipped,
            const ui::ClipEvent& event)
        {
            IItem::clipEvent(clipRect, clipped, event);
            TLRENDER_P();
            if (clipped)
            {
                p.draw.labelGlyphs.clear();
                p.draw.durationGlyphs.clear();
            }
        }

        void AudioGapItem::drawEvent(
            const math::BBox2i& drawRect,
            const ui::DrawEvent& event)
        {
            IItem::drawEvent(drawRect, event);
            TLRENDER_P();

            const auto fontInfo = event.style->getFontRole(p.fontRole, event.displayScale);
            const auto fontMetrics = event.getFontMetrics(p.fontRole);
            const math::BBox2i& g = _geometry;

            const math::BBox2i g2 = g.margin(-p.size.border);
            event.render->drawMesh(
                ui::rect(g2, p.size.margin),
                math::Vector2i(),
                imaging::Color4f(.25F, .24F, .3F));

            const math::BBox2i labelGeometry(
                g.min.x +
                p.size.margin,
                g.min.y +
                p.size.margin,
                p.size.labelSize.x,
                p.size.labelSize.y);
            const math::BBox2i durationGeometry(
                g.max.x -
                p.size.margin -
                p.size.durationSize.x,
                g.min.y +
                p.size.margin,
                p.size.durationSize.x,
                p.size.durationSize.y);
            const bool labelVisible = drawRect.intersects(labelGeometry);
            const bool durationVisible =
                drawRect.intersects(durationGeometry) &&
                !durationGeometry.intersects(labelGeometry);

            if (labelVisible)
            {
                event.render->drawText(
                    event.fontSystem->getGlyphs(p.label, fontInfo),
                    math::Vector2i(
                        labelGeometry.min.x,
                        labelGeometry.min.y +
                        fontMetrics.ascender),
                    event.style->getColorRole(ui::ColorRole::Text));
            }

            if (durationVisible)
            {
                event.render->drawText(
                    event.fontSystem->getGlyphs(p.durationLabel, fontInfo),
                    math::Vector2i(
                        durationGeometry.min.x,
                        durationGeometry.min.y +
                        fontMetrics.ascender),
                    event.style->getColorRole(ui::ColorRole::Text));
            }
        }

        void AudioGapItem::_textUpdate()
        {
            TLRENDER_P();
            p.durationLabel = IItem::_durationLabel(
                p.timeRange.duration(),
                _options.timeUnits);
            p.draw.durationGlyphs.clear();
            _updates |= ui::Update::Size;
            _updates |= ui::Update::Draw;
        }

        std::string AudioGapItem::_nameLabel(const std::string& name)
        {
            return !name.empty() ?
                name :
                std::string("Gap");
        }
    }
}
