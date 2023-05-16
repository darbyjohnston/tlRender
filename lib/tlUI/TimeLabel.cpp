// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlUI/TimeLabel.h>

#include <tlUI/LayoutUtil.h>

#include <tlTimeline/TimeUnits.h>

#include <tlCore/StringFormat.h>

namespace tl
{
    namespace ui
    {
        struct TimeLabel::Private
        {
            std::shared_ptr<timeline::TimeUnitsModel> timeUnitsModel;
            otime::RationalTime value = time::invalidTime;
            std::string text;
            std::string format;
            SizeRole marginRole = SizeRole::None;
            FontRole fontRole = FontRole::Label;

            struct SizeData
            {
                int margin = 0;
                imaging::FontMetrics fontMetrics;
                math::Vector2i textSize;
                math::Vector2i formatSize;
            };
            SizeData size;

            struct DrawData
            {
                std::vector<std::shared_ptr<imaging::Glyph> > glyphs;
            };
            DrawData draw;

            std::shared_ptr<observer::ValueObserver<timeline::TimeUnits> > timeUnitsObserver;
        };

        void TimeLabel::_init(
            const std::shared_ptr<timeline::TimeUnitsModel>& timeUnitsModel,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init("tl::ui::TimeLabel", context, parent);
            TLRENDER_P();

            p.timeUnitsModel = timeUnitsModel;
            if (!p.timeUnitsModel)
            {
                p.timeUnitsModel = timeline::TimeUnitsModel::create(context);
            }

            _textUpdate();

            p.timeUnitsObserver = observer::ValueObserver<timeline::TimeUnits>::create(
                p.timeUnitsModel->observeTimeUnits(),
                [this](timeline::TimeUnits)
                {
                    _textUpdate();
                });
        }

        TimeLabel::TimeLabel() :
            _p(new Private)
        {}

        TimeLabel::~TimeLabel()
        {}

        std::shared_ptr<TimeLabel> TimeLabel::create(
            const std::shared_ptr<timeline::TimeUnitsModel>& timeUnitsModel,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<TimeLabel>(new TimeLabel);
            out->_init(timeUnitsModel, context, parent);
            return out;
        }

        const std::shared_ptr<timeline::TimeUnitsModel>& TimeLabel::getTimeUnitsModel() const
        {
            return _p->timeUnitsModel;
        }

        const otime::RationalTime& TimeLabel::getValue() const
        {
            return _p->value;
        }

        void TimeLabel::setValue(const otime::RationalTime& value)
        {
            TLRENDER_P();
            if (time::compareExact(value, p.value))
                return;
            p.value = value;
            _textUpdate();
        }

        void TimeLabel::setMarginRole(SizeRole value)
        {
            TLRENDER_P();
            if (value == p.marginRole)
                return;
            p.marginRole = value;
            _updates |= Update::Size;
            _updates |= Update::Draw;
        }

        void TimeLabel::setFontRole(FontRole value)
        {
            TLRENDER_P();
            if (value == p.fontRole)
                return;
            p.fontRole = value;
            p.draw.glyphs.clear();
            _updates |= Update::Size;
            _updates |= Update::Draw;
        }

        void TimeLabel::sizeHintEvent(const SizeHintEvent& event)
        {
            IWidget::sizeHintEvent(event);
            TLRENDER_P();
            p.size.margin = event.style->getSizeRole(p.marginRole, event.displayScale);
            p.size.fontMetrics = event.getFontMetrics(p.fontRole);
            const auto fontInfo = event.style->getFontRole(p.fontRole, event.displayScale);
            p.size.textSize = event.fontSystem->getSize(p.text, fontInfo);
            p.size.formatSize = event.fontSystem->getSize(p.format, fontInfo);
            _sizeHint.x =
                std::max(p.size.textSize.x, p.size.formatSize.x) +
                p.size.margin * 2;
            _sizeHint.y =
                p.size.fontMetrics.lineHeight +
                p.size.margin * 2;
        }

        void TimeLabel::clipEvent(
            const math::BBox2i& clipRect,
            bool clipped,
            const ClipEvent& event)
        {
            IWidget::clipEvent(clipRect, clipped, event);
            TLRENDER_P();
            if (clipped)
            {
                p.draw.glyphs.clear();
            }
        }

        void TimeLabel::drawEvent(
            const math::BBox2i& drawRect,
            const DrawEvent& event)
        {
            IWidget::drawEvent(drawRect, event);
            TLRENDER_P();

            //event.render->drawRect(_geometry, imaging::Color4f(.5F, .3F, .3F));

            const math::BBox2i g = align(
                _geometry,
                _sizeHint,
                Stretch::Fixed,
                Stretch::Fixed,
                _hAlign,
                _vAlign).margin(-p.size.margin);

            if (!p.text.empty() && p.draw.glyphs.empty())
            {
                const auto fontInfo = event.style->getFontRole(p.fontRole, event.displayScale);
                p.draw.glyphs = event.fontSystem->getGlyphs(p.text, fontInfo);
            }
            const math::Vector2i pos(
                g.x(),
                g.y() + p.size.fontMetrics.ascender);
            event.render->drawText(
                p.draw.glyphs,
                pos,
                event.style->getColorRole(ColorRole::Text));
        }

        void TimeLabel::_textUpdate()
        {
            TLRENDER_P();
            p.text = std::string();
            p.format = std::string();
            if (p.timeUnitsModel)
            {
                switch (p.timeUnitsModel->getTimeUnits())
                {
                case timeline::TimeUnits::Frames:
                    p.text = string::Format("{0}").arg(p.value.to_frames());
                    p.format = "000000";
                    break;
                case timeline::TimeUnits::Seconds:
                    p.text = string::Format("{0}").arg(p.value.to_seconds(), 2);
                    p.format = "000000.00";
                    break;
                case timeline::TimeUnits::Timecode:
                    p.text = p.value.to_timecode();
                    p.format = "00:00:00;00";
                    break;
                default: break;
                }
            }
            p.draw.glyphs.clear();
            _updates |= Update::Size;
            _updates |= Update::Draw;
        }
    }
}
