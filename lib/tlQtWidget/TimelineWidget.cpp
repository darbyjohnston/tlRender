// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlQtWidget/TimelineWidget.h>

#include <tlQtWidget/Util.h>

#include <tlTimelineUI/TimelineWidget.h>
#include <tlTimelineUI/TimelineWidget.h>

#include <tlTimeline/Edit.h>

#include <QAction>
#include <QContextMenuEvent>
#include <QMenu>

namespace tl
{
    namespace qtwidget
    {
        struct TimelineWidget::Private
        {
            int currentTrack = -1;

            std::shared_ptr<timelineui::TimelineWidget> timelineWidget;
            QAction* trackEnabledAction = nullptr;

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

            p.trackEnabledAction = new QAction(this);
            p.trackEnabledAction->setText("Track enabled");
            p.trackEnabledAction->setCheckable(true);

            connect(
                p.trackEnabledAction,
                SIGNAL(toggled(bool)),
                SLOT(_trackEnabledCallback(bool)));

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

        std::shared_ptr<timeline::Player>& TimelineWidget::player() const
        {
            return _p->timelineWidget->getPlayer();
        }

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

        void TimelineWidget::contextMenuEvent(QContextMenuEvent* event)
        {
            TLRENDER_P();
            if (auto player = p.timelineWidget->getPlayer())
            {
                const math::Vector2i pos = _toUI(math::Vector2i(event->x(), event->y()));
                const std::vector<math::Box2i> trackGeom = p.timelineWidget->getTrackGeom();
                for (int i = 0; i < trackGeom.size(); ++i)
                {
                    if (trackGeom[i].contains(pos))
                    {
                        p.currentTrack = i;

                        bool checked = false;
                        auto otioTimeline = player->getTimeline()->getTimeline();
                        const auto& children = otioTimeline->tracks()->children();
                        if (i >= 0 && i < children.size())
                        {
                            if (auto track = otio::dynamic_retainer_cast<otio::Item>(children[i]))
                            {
                                checked = track->enabled();
                            }
                        }
                        p.trackEnabledAction->setChecked(checked);

                        QMenu menu(this);
                        menu.addAction(p.trackEnabledAction);
                        menu.exec(event->globalPos());
                        break;
                    }
                }
            }
        }

        void TimelineWidget::_trackEnabledCallback(bool value)
        {
            TLRENDER_P();
            if (auto player = p.timelineWidget->getPlayer())
            {
                auto otioTimeline = player->getTimeline()->getTimeline();
                auto children = otioTimeline->tracks()->children();
                if (p.currentTrack >= 0 && p.currentTrack < children.size())
                {
                    if (auto track = otio::dynamic_retainer_cast<otio::Item>(children[p.currentTrack]))
                    {
                        if (value != track->enabled())
                        {
                            auto otioTimelineNew = timeline::copy(player->getTimeline()->getTimeline().value);
                            children = otioTimelineNew->tracks()->children();
                            track = otio::dynamic_retainer_cast<otio::Item>(children[p.currentTrack]);
                            track->set_enabled(value);
                            player->getTimeline()->setTimeline(otioTimelineNew);
                        }
                    }
                }
            }
        }
    }
}
