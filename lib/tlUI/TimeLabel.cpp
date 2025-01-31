// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlUI/TimeLabel.h>

#include <tlUI/LayoutUtil.h>

#include <tlTimeline/TimeUnits.h>

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
                bool sizeInit = true;
                int margin = 0;

                bool textInit = true;
                image::FontInfo fontInfo;
                image::FontMetrics fontMetrics;
                math::Size2i textSize;
                math::Size2i formatSize;
            };
            SizeData size;

            struct DrawData
            {
                std::vector<std::shared_ptr<image::Glyph> > glyphs;
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
            if (value.strictly_equal(p.value))
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
            p.size.sizeInit = true;
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
            const bool displayScaleChanged = event.displayScale != _displayScale;
            IWidget::sizeHintEvent(event);
            TLRENDER_P();

            if (displayScaleChanged || p.size.sizeInit)
            {
                p.size.margin = event.style->getSizeRole(p.marginRole, _displayScale);
            }
            if (displayScaleChanged || p.size.textInit || p.size.sizeInit)
            {
                p.size.fontInfo = event.style->getFontRole(p.fontRole, _displayScale);
                p.size.fontMetrics = event.fontSystem->getMetrics(p.size.fontInfo);
                p.size.textSize = event.fontSystem->getSize(p.text, p.size.fontInfo);
                p.size.formatSize = event.fontSystem->getSize(p.format, p.size.fontInfo);
                p.draw.glyphs.clear();
            }
            p.size.sizeInit = false;
            p.size.textInit = false;

            _sizeHint.w =
                std::max(p.size.textSize.w, p.size.formatSize.w) +
                p.size.margin * 2;
            _sizeHint.h =
                p.size.fontMetrics.lineHeight +
                p.size.margin * 2;
        }

        void TimeLabel::clipEvent(const math::Box2i& clipRect, bool clipped)
        {
            IWidget::clipEvent(clipRect, clipped);
            TLRENDER_P();
            if (clipped)
            {
                p.draw.glyphs.clear();
            }
        }

        void TimeLabel::drawEvent(
            const math::Box2i& drawRect,
            const DrawEvent& event)
        {
            IWidget::drawEvent(drawRect, event);
            TLRENDER_P();

            //event.render->drawRect(_geometry, image::Color4f(.5F, .3F, .3F));

            const math::Box2i g = align(
                _geometry,
                _sizeHint,
                Stretch::Fixed,
                Stretch::Fixed,
                _hAlign,
                _vAlign).margin(-p.size.margin);

            if (!p.text.empty() && p.draw.glyphs.empty())
            {
                p.draw.glyphs = event.fontSystem->getGlyphs(p.text, p.size.fontInfo);
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
                const timeline::TimeUnits timeUnits = p.timeUnitsModel->getTimeUnits();
                p.text = timeline::timeToText(p.value, timeUnits);
                p.format = timeline::formatString(timeUnits);
            }
            p.size.textInit = true;
            _updates |= Update::Size;
            _updates |= Update::Draw;
        }
    }
}
