// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include "AudioClipItem.h"

#include <tlUI/DrawUtil.h>

#include <QPainter>

namespace tl
{
    namespace examples
    {
        namespace timeline_qtwidget
        {
            void  AudioClipItem::_init(
                const otio::Clip* clip,
                const std::shared_ptr<timeline::Timeline>& timeline,
                const std::shared_ptr<system::Context>& context,
                const std::shared_ptr<IWidget>& parent)
            {
                IItem::_init("AudioClipItem", timeline, context, parent);

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

            AudioClipItem::~AudioClipItem()
            {
                _cancelAudioRequests();
            }

            std::shared_ptr<AudioClipItem>  AudioClipItem::create(
                const otio::Clip* clip,
                const std::shared_ptr<timeline::Timeline>& timeline,
                const std::shared_ptr<system::Context>& context,
                const std::shared_ptr<IWidget>& parent)
            {
                auto out = std::shared_ptr<AudioClipItem>(new AudioClipItem);
                out->_init(clip, timeline, context, parent);
                return out;
            }

            void AudioClipItem::setScale(float value)
            {
                IItem::setScale(value);
                if (_updates & ui::Update::Size)
                {
                    _cancelAudioRequests();
                }
            }

            void AudioClipItem::setThumbnailHeight(int value)
            {
                IItem::setThumbnailHeight(value);
                if (_updates & ui::Update::Size)
                {
                    _cancelAudioRequests();
                }
            }

            void AudioClipItem::setViewport(const math::BBox2i& value)
            {
                IItem::setViewport(value);
                if (_updates & ui::Update::Size)
                {
                    _cancelAudioRequests();
                }
            }

            void AudioClipItem::tickEvent(const ui::TickEvent& event)
            {
            }

            void AudioClipItem::sizeEvent(const ui::SizeEvent& event)
            {
                IItem::sizeEvent(event);

                _margin = event.style->getSizeRole(ui::SizeRole::MarginSmall) * event.contentScale;
                _spacing = event.style->getSizeRole(ui::SizeRole::SpacingSmall) * event.contentScale;
                _border = event.style->getSizeRole(ui::SizeRole::Border) * event.contentScale;
                auto fontInfo = _fontInfo;
                fontInfo.size *= event.contentScale;
                _fontMetrics = event.fontSystem->getMetrics(fontInfo);

                _sizeHint = math::Vector2i(
                    _timeRange.duration().rescaled_to(1.0).value() * _scale,
                    _margin +
                    _fontMetrics.lineHeight +
                    _spacing +
                    _fontMetrics.lineHeight +
                    _margin);
            }

            void AudioClipItem::drawEvent(const ui::DrawEvent& event)
            {
                IItem::drawEvent(event);

                auto fontInfo = _fontInfo;
                fontInfo.size *= event.contentScale;
                math::BBox2i g = _geometry;
                g.min = g.min - _viewport.min;
                g.max = g.max - _viewport.min;

                event.render->drawMesh(
                    ui::border(g, _border, _margin / 2),
                    event.style->getColorRole(ui::ColorRole::Border));

                event.render->drawRect(
                    g.margin(-_border),
                    imaging::Color4f(.2F, .4F, .2F));

                event.render->drawText(
                    event.fontSystem->getGlyphs(_label, fontInfo),
                    math::Vector2i(
                        g.min.x +
                        _margin,
                        g.min.y +
                        _margin +
                        _fontMetrics.ascender),
                    event.style->getColorRole(ui::ColorRole::Text));
                event.render->drawText(
                    event.fontSystem->getGlyphs(_startLabel, fontInfo),
                    math::Vector2i(
                        g.min.x +
                        _margin,
                        g.min.y +
                        _margin +
                        _fontMetrics.lineHeight +
                        _spacing +
                        _fontMetrics.ascender),
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
                        _fontMetrics.ascender),
                    event.style->getColorRole(ui::ColorRole::Text));
                textSize = event.fontSystem->measure(_endLabel, fontInfo);
                event.render->drawText(
                    event.fontSystem->getGlyphs(_endLabel, fontInfo),
                    math::Vector2i(
                        g.max.x -
                        _margin -
                        textSize.x,
                        g.min.y +
                        _margin +
                        _fontMetrics.lineHeight +
                        _spacing +
                        _fontMetrics.ascender),
                    event.style->getColorRole(ui::ColorRole::Text));
            }

            std::string AudioClipItem::_nameLabel(const std::string& name)
            {
                return !name.empty() ?
                    name :
                    std::string("Clip");
            }

            void AudioClipItem::_cancelAudioRequests()
            {}
        }
    }
}
