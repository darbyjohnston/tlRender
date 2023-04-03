// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include "AudioGapItem.h"

#include <tlUI/DrawUtil.h>

namespace tl
{
    namespace examples
    {
        namespace timeline_qtwidget
        {
            void AudioGapItem::_init(
                const otio::Gap* gap,
                const ItemData& itemData,
                const std::shared_ptr<system::Context>& context,
                const std::shared_ptr<IWidget>& parent)
            {
                IItem::_init("AudioGapItem", itemData, context, parent);

                auto rangeOpt = gap->trimmed_range_in_parent();
                if (rangeOpt.has_value())
                {
                    _timeRange = rangeOpt.value();
                }

                _label = _nameLabel(gap->name());
                _textUpdate();
            }

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

            void AudioGapItem::sizeEvent(const ui::SizeEvent& event)
            {
                IItem::sizeEvent(event);

                _margin = event.style->getSizeRole(ui::SizeRole::MarginSmall) * event.contentScale;
                const auto fontMetrics = event.getFontMetrics(_fontRole);

                _sizeHint = math::Vector2i(
                    _timeRange.duration().rescaled_to(1.0).value() * _options.scale,
                    _margin +
                    fontMetrics.lineHeight +
                    _margin);
            }

            void AudioGapItem::drawEvent(const ui::DrawEvent& event)
            {
                IItem::drawEvent(event);
                if (_insideViewport())
                {
                    const int b = event.style->getSizeRole(ui::SizeRole::Border) * event.contentScale;
                    const auto fontInfo = event.getFontInfo(_fontRole);
                    const auto fontMetrics = event.getFontMetrics(_fontRole);
                    math::BBox2i g = _geometry;

                    //event.render->drawMesh(
                    //    ui::border(g, b, _margin / 2),
                    //    event.style->getColorRole(ui::ColorRole::Border));

                    //event.render->drawRect(
                    //    g.margin(-b),
                    //    imaging::Color4f(.2F, .2F, .25F));

                    event.render->drawText(
                        event.fontSystem->getGlyphs(_label, fontInfo),
                        math::Vector2i(
                            g.min.x +
                            _margin,
                            g.min.y +
                            _margin +
                            fontMetrics.ascender),
                        event.style->getColorRole(ui::ColorRole::Text));

                    math::Vector2i textSize = event.fontSystem->measure(_durationLabel, fontInfo);
                    event.render->drawText(
                        event.fontSystem->getGlyphs(_durationLabel, fontInfo),
                        math::Vector2i(
                            g.max.x -
                            _margin -
                            textSize.x,
                            g.min.y +
                            _margin +
                            fontMetrics.ascender),
                        event.style->getColorRole(ui::ColorRole::Text));
                }
            }

            void AudioGapItem::_textUpdate()
            {
                _durationLabel = IItem::_durationLabel(
                    _timeRange.duration(),
                    _options.timeUnits);
            }

            std::string AudioGapItem::_nameLabel(const std::string& name)
            {
                return !name.empty() ?
                    name :
                    std::string("Gap");
            }
        }
    }
}
