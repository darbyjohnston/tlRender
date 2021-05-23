// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrQt/TimelineWidget.h>

#include <QVBoxLayout>

namespace tlr
{
    namespace qt
    {
        TimelineWidget::TimelineWidget(QWidget* parent) :
            QWidget(parent)
        {
            _viewport = new TimelineViewport;

            _slider = new TimelineSlider;
            _slider->setToolTip(tr("Timeline slider"));

            _controls = new TimelineControls;

            auto layout = new QVBoxLayout;
            layout->setMargin(0);
            layout->setSpacing(0);
            layout->addWidget(_viewport, 1);
            auto vLayout = new QVBoxLayout;
            vLayout->setMargin(5);
            vLayout->setSpacing(5);
            vLayout->addWidget(_slider, 1);
            vLayout->addWidget(_controls);
            layout->addLayout(vLayout);
            setLayout(layout);
        }

        void TimelineWidget::setTimeObject(TimeObject* timeObject)
        {
            _slider->setTimeObject(timeObject);
            _controls->setTimeObject(timeObject);
        }

        void TimelineWidget::setTimelinePlayer(TimelinePlayer* timelinePlayer)
        {
            _viewport->setTimelinePlayer(timelinePlayer);
            _slider->setTimelinePlayer(timelinePlayer);
            _controls->setTimelinePlayer(timelinePlayer);
        }
    }
}
