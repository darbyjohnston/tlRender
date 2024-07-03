// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlQtWidget/TimelineWidget.h>

#include <tlQtWidget/Util.h>

#include <tlTimelineUI/TimelineWidget.h>

namespace tl
{
    namespace qtwidget
    {
        struct TimelineWidget::Private
        {
            std::shared_ptr<timelineui::TimelineWidget> timelineWidget;

            std::shared_ptr<observer::ValueObserver<bool> > editableObserver;
            std::shared_ptr<observer::ValueObserver<bool> > frameViewObserver;
            std::shared_ptr<observer::ValueObserver<bool> > scrubObserver;
            std::shared_ptr<observer::ValueObserver<otime::RationalTime> > timeScrubObserver;
        };

        TimelineWidget::TimelineWidget(
            const std::shared_ptr<timeline::ITimeUnitsModel>& timeUnitsModel,
            const std::shared_ptr<ui::Style>& style,
            const std::shared_ptr<system::Context>& context,
            QWidget* parent) :
            ContainerWidget(style, context, parent),
            _p(new Private)
        {
            TLRENDER_P();

            p.timelineWidget = timelineui::TimelineWidget::create(timeUnitsModel, context);
            //p.timelineWidget->setScrollBarsVisible(false);
            setWidget(p.timelineWidget);

            p.editableObserver = observer::ValueObserver<bool>::create(
                p.timelineWidget->observeEditable(),
                [this](bool value)
                {
                    Q_EMIT editableChanged(value);
                });

            p.frameViewObserver = observer::ValueObserver<bool>::create(
                p.timelineWidget->observeFrameView(),
                [this](bool value)
                {
                    Q_EMIT frameViewChanged(value);
                });

            p.scrubObserver = observer::ValueObserver<bool>::create(
                p.timelineWidget->observeScrub(),
                [this](bool value)
                {
                    Q_EMIT scrubChanged(value);
                });

            p.timeScrubObserver = observer::ValueObserver<otime::RationalTime>::create(
                p.timelineWidget->observeTimeScrub(),
                [this](const otime::RationalTime& value)
                {
                    Q_EMIT timeScrubbed(value);
                });
        }

        TimelineWidget::~TimelineWidget()
        {}

        void TimelineWidget::setPlayer(const std::shared_ptr<timeline::Player>& player)
        {
            _p->timelineWidget->setPlayer(player);
        }

        bool TimelineWidget::isEditable() const
        {
            return _p->timelineWidget->isEditable();
        }

        bool TimelineWidget::hasFrameView() const
        {
            return _p->timelineWidget->hasFrameView();
        }

        bool TimelineWidget::areScrollBarsVisible() const
        {
            return _p->timelineWidget->areScrollBarsVisible();
        }

        bool TimelineWidget::hasScrollToCurrentFrame() const
        {
            return _p->timelineWidget->hasScrollToCurrentFrame();
        }

        ui::KeyModifier TimelineWidget::scrollKeyModifier() const
        {
            return _p->timelineWidget->getScrollKeyModifier();
        }

        float TimelineWidget::mouseWheelScale() const
        {
            return _p->timelineWidget->getMouseWheelScale();
        }

        bool TimelineWidget::hasStopOnScrub() const
        {
            return _p->timelineWidget->hasStopOnScrub();
        }

        const std::vector<int>& TimelineWidget::frameMarkers() const
        {
            return _p->timelineWidget->getFrameMarkers();
        }

        const timelineui::ItemOptions& TimelineWidget::itemOptions() const
        {
            return _p->timelineWidget->getItemOptions();
        }

        const timelineui::DisplayOptions& TimelineWidget::displayOptions() const
        {
            return _p->timelineWidget->getDisplayOptions();
        }

        void TimelineWidget::setEditable(bool value)
        {
            _p->timelineWidget->setEditable(value);
        }

        void TimelineWidget::setFrameView(bool value)
        {
            _p->timelineWidget->setFrameView(value);
        }

        void TimelineWidget::setScrollBarsVisible(bool value)
        {
            _p->timelineWidget->setScrollBarsVisible(value);
        }

        void TimelineWidget::setScrollToCurrentFrame(bool value)
        {
            _p->timelineWidget->setScrollToCurrentFrame(value);
        }

        void TimelineWidget::setScrollKeyModifier(ui::KeyModifier value)
        {
            _p->timelineWidget->setScrollKeyModifier(value);
        }

        void TimelineWidget::setMouseWheelScale(float value)
        {
            _p->timelineWidget->setMouseWheelScale(value);
        }

        void TimelineWidget::setStopOnScrub(bool value)
        {
            _p->timelineWidget->setStopOnScrub(value);
        }

        void TimelineWidget::setFrameMarkers(const std::vector<int>& value)
        {
            _p->timelineWidget->setFrameMarkers(value);
        }

        void TimelineWidget::setItemOptions(const timelineui::ItemOptions& value)
        {
            TLRENDER_P();
            p.timelineWidget->setItemOptions(value);
            setInputEnabled(value.inputEnabled);
        }

        void TimelineWidget::setDisplayOptions(const timelineui::DisplayOptions& value)
        {
            _p->timelineWidget->setDisplayOptions(value);
        }
    }
}
