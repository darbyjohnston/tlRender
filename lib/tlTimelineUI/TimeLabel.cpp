// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlTimelineUI/TimeLabel.h>

#include <tlTimeline/TimeUnits.h>

#include <dtk/ui/LayoutUtil.h>

namespace tl
{
    namespace timelineui
    {
        struct TimeLabel::Private
        {
            std::shared_ptr<timeline::TimeUnitsModel> timeUnitsModel;
            OTIO_NS::RationalTime value = time::invalidTime;
            std::string text;
            std::string format;
            dtk::SizeRole marginRole = dtk::SizeRole::None;
            dtk::FontRole fontRole = dtk::FontRole::Label;

            struct SizeData
            {
                bool init = true;
                float displayScale = 0.F;
                int margin = 0;

                bool textInit = true;
                dtk::FontInfo fontInfo;
                dtk::FontMetrics fontMetrics;
                dtk::Size2I textSize;
                dtk::Size2I formatSize;
            };
            SizeData size;

            struct DrawData
            {
                std::vector<std::shared_ptr<dtk::Glyph> > glyphs;
            };
            DrawData draw;

            std::shared_ptr<dtk::ValueObserver<timeline::TimeUnits> > timeUnitsObserver;
        };

        void TimeLabel::_init(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<timeline::TimeUnitsModel>& timeUnitsModel,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init(context, "tl::timelineui::TimeLabel", parent);
            DTK_P();

            p.timeUnitsModel = timeUnitsModel;
            if (!p.timeUnitsModel)
            {
                p.timeUnitsModel = timeline::TimeUnitsModel::create(context);
            }

            _textUpdate();

            p.timeUnitsObserver = dtk::ValueObserver<timeline::TimeUnits>::create(
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
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<timeline::TimeUnitsModel>& timeUnitsModel,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<TimeLabel>(new TimeLabel);
            out->_init(context, timeUnitsModel, parent);
            return out;
        }

        const std::shared_ptr<timeline::TimeUnitsModel>& TimeLabel::getTimeUnitsModel() const
        {
            return _p->timeUnitsModel;
        }

        const OTIO_NS::RationalTime& TimeLabel::getValue() const
        {
            return _p->value;
        }

        void TimeLabel::setValue(const OTIO_NS::RationalTime& value)
        {
            DTK_P();
            if (value.strictly_equal(p.value))
                return;
            p.value = value;
            _textUpdate();
        }

        void TimeLabel::setMarginRole(dtk::SizeRole value)
        {
            DTK_P();
            if (value == p.marginRole)
                return;
            p.marginRole = value;
            p.size.init = true;
            _setSizeUpdate();
            _setDrawUpdate();
        }

        void TimeLabel::setFontRole(dtk::FontRole value)
        {
            DTK_P();
            if (value == p.fontRole)
                return;
            p.fontRole = value;
            p.size.textInit = true;
            p.draw.glyphs.clear();
            _setSizeUpdate();
            _setDrawUpdate();
        }

        void TimeLabel::sizeHintEvent(const dtk::SizeHintEvent& event)
        {
            IWidget::sizeHintEvent(event);
            DTK_P();

            if (p.size.init || event.displayScale != p.size.displayScale)
            {
                p.size.margin = event.style->getSizeRole(p.marginRole, event.displayScale);
            }
            if (p.size.init || event.displayScale != p.size.displayScale || p.size.textInit)
            {
                p.size.fontInfo = event.style->getFontRole(p.fontRole, event.displayScale);
                p.size.fontMetrics = event.fontSystem->getMetrics(p.size.fontInfo);
                p.size.textSize = event.fontSystem->getSize(p.text, p.size.fontInfo);
                p.size.formatSize = event.fontSystem->getSize(p.format, p.size.fontInfo);
                p.draw.glyphs.clear();
            }
            p.size.init = false;
            p.size.displayScale = event.displayScale;
            p.size.textInit = false;

            dtk::Size2I sizeHint;
            sizeHint.w =
                std::max(p.size.textSize.w, p.size.formatSize.w) +
                p.size.margin * 2;
            sizeHint.h =
                p.size.fontMetrics.lineHeight +
                p.size.margin * 2;
            _setSizeHint(sizeHint);
        }

        void TimeLabel::clipEvent(const dtk::Box2I& clipRect, bool clipped)
        {
            IWidget::clipEvent(clipRect, clipped);
            DTK_P();
            if (clipped)
            {
                p.draw.glyphs.clear();
            }
        }

        void TimeLabel::drawEvent(
            const dtk::Box2I& drawRect,
            const dtk::DrawEvent& event)
        {
            IWidget::drawEvent(drawRect, event);
            DTK_P();

            //event.render->drawRect(_geometry, dtk::Color4F(.5F, .3F, .3F));

            const dtk::Box2I g = dtk::margin(
                align(
                    getGeometry(),
                    getSizeHint(),
                    getHAlign(),
                    getVAlign()),
                -p.size.margin);

            if (!p.text.empty() && p.draw.glyphs.empty())
            {
                p.draw.glyphs = event.fontSystem->getGlyphs(p.text, p.size.fontInfo);
            }
            event.render->drawText(
                p.draw.glyphs,
                p.size.fontMetrics,
                g.min,
                event.style->getColorRole(dtk::ColorRole::Text));
        }

        void TimeLabel::_textUpdate()
        {
            DTK_P();
            p.text = std::string();
            p.format = std::string();
            if (p.timeUnitsModel)
            {
                const timeline::TimeUnits timeUnits = p.timeUnitsModel->getTimeUnits();
                p.text = timeline::timeToText(p.value, timeUnits);
                p.format = timeline::formatString(timeUnits);
            }
            p.size.textInit = true;
            _setSizeUpdate();
            _setDrawUpdate();
        }
    }
}
