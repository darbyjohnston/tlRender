// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlTimelineUI/TimeLabel.h>

#include <tlTimeline/TimeUnits.h>

#include <feather-tk/ui/LayoutUtil.h>

#include <optional>

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
            feather_tk::SizeRole marginRole = feather_tk::SizeRole::None;
            feather_tk::FontRole fontRole = feather_tk::FontRole::Label;

            struct SizeData
            {
                std::optional<float> displayScale;
                int margin = 0;
                feather_tk::FontInfo fontInfo;
                feather_tk::FontMetrics fontMetrics;
                feather_tk::Size2I textSize;
                feather_tk::Size2I formatSize;
            };
            SizeData size;

            struct DrawData
            {
                std::vector<std::shared_ptr<feather_tk::Glyph> > glyphs;
            };
            std::optional<DrawData> draw;

            std::shared_ptr<feather_tk::ValueObserver<timeline::TimeUnits> > timeUnitsObserver;
        };

        void TimeLabel::_init(
            const std::shared_ptr<feather_tk::Context>& context,
            const std::shared_ptr<timeline::TimeUnitsModel>& timeUnitsModel,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init(context, "tl::timelineui::TimeLabel", parent);
            FEATHER_TK_P();

            setVAlign(feather_tk::VAlign::Center);

            p.timeUnitsModel = timeUnitsModel;
            if (!p.timeUnitsModel)
            {
                p.timeUnitsModel = timeline::TimeUnitsModel::create(context);
            }

            _textUpdate();

            p.timeUnitsObserver = feather_tk::ValueObserver<timeline::TimeUnits>::create(
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
            const std::shared_ptr<feather_tk::Context>& context,
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
            FEATHER_TK_P();
            if (value.strictly_equal(p.value))
                return;
            p.value = value;
            _textUpdate();
        }

        void TimeLabel::setMarginRole(feather_tk::SizeRole value)
        {
            FEATHER_TK_P();
            if (value == p.marginRole)
                return;
            p.marginRole = value;
            p.size.displayScale.reset();
            _setSizeUpdate();
            _setDrawUpdate();
        }

        void TimeLabel::setFontRole(feather_tk::FontRole value)
        {
            FEATHER_TK_P();
            if (value == p.fontRole)
                return;
            p.fontRole = value;
            p.size.displayScale.reset();
            _setSizeUpdate();
            _setDrawUpdate();
        }

        void TimeLabel::sizeHintEvent(const feather_tk::SizeHintEvent& event)
        {
            IWidget::sizeHintEvent(event);
            FEATHER_TK_P();

            if (!p.size.displayScale.has_value() ||
                (p.size.displayScale.has_value() && p.size.displayScale.value() != event.displayScale))
            {
                p.size.displayScale = event.displayScale;
                p.size.margin = event.style->getSizeRole(p.marginRole, event.displayScale);
                p.size.fontInfo = event.style->getFontRole(p.fontRole, event.displayScale);
                p.size.fontMetrics = event.fontSystem->getMetrics(p.size.fontInfo);
                p.size.textSize = event.fontSystem->getSize(p.text, p.size.fontInfo);
                p.size.formatSize = event.fontSystem->getSize(p.format, p.size.fontInfo);
                p.draw.reset();
            }

            feather_tk::Size2I sizeHint;
            sizeHint.w =
                std::max(p.size.textSize.w, p.size.formatSize.w) +
                p.size.margin * 2;
            sizeHint.h =
                p.size.fontMetrics.lineHeight +
                p.size.margin * 2;
            _setSizeHint(sizeHint);
        }

        void TimeLabel::clipEvent(const feather_tk::Box2I& clipRect, bool clipped)
        {
            IWidget::clipEvent(clipRect, clipped);
            FEATHER_TK_P();
            if (clipped)
            {
                p.draw.reset();
            }
        }

        void TimeLabel::drawEvent(
            const feather_tk::Box2I& drawRect,
            const feather_tk::DrawEvent& event)
        {
            IWidget::drawEvent(drawRect, event);
            FEATHER_TK_P();

            if (!p.draw.has_value())
            {
                p.draw = Private::DrawData();
            }

            const feather_tk::Box2I g = feather_tk::margin(
                align(
                    getGeometry(),
                    getSizeHint(),
                    getHAlign(),
                    getVAlign()),
                -p.size.margin);

            if (!p.text.empty() && p.draw->glyphs.empty())
            {
                p.draw->glyphs = event.fontSystem->getGlyphs(p.text, p.size.fontInfo);
            }
            event.render->drawText(
                p.draw->glyphs,
                p.size.fontMetrics,
                g.min,
                event.style->getColorRole(feather_tk::ColorRole::Text));
        }

        void TimeLabel::_textUpdate()
        {
            FEATHER_TK_P();
            p.text = std::string();
            p.format = std::string();
            if (p.timeUnitsModel)
            {
                const timeline::TimeUnits timeUnits = p.timeUnitsModel->getTimeUnits();
                p.text = timeline::timeToText(p.value, timeUnits);
                p.format = timeline::formatString(timeUnits);
            }
            p.size.displayScale.reset();
            _setSizeUpdate();
            _setDrawUpdate();
        }
    }
}
