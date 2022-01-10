// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlrQWidget/TimelineWidget.h>

#include <tlrQWidget/TimelineControls.h>
#include <tlrQWidget/TimelineSlider.h>
#include <tlrQWidget/TimelineViewport.h>

#include <QVBoxLayout>

namespace tlr
{
    namespace qwidget
    {
        struct TimelineWidget::Private
        {
            TimelineViewport* viewport = nullptr;
            TimelineSlider* slider = nullptr;
            TimelineControls* controls = nullptr;
        };

        TimelineWidget::TimelineWidget(
            const std::shared_ptr<core::Context>& context,
            QWidget* parent) :
            QWidget(parent),
            _p(new Private)
        {
            TLR_PRIVATE_P();

            p.viewport = new TimelineViewport(context);

            p.slider = new TimelineSlider;
            p.slider->setToolTip(tr("Timeline slider"));

            p.controls = new TimelineControls;

            auto layout = new QVBoxLayout;
            layout->setMargin(0);
            layout->setSpacing(0);
            layout->addWidget(p.viewport, 1);
            auto vLayout = new QVBoxLayout;
            vLayout->setMargin(5);
            vLayout->setSpacing(5);
            vLayout->addWidget(p.slider, 1);
            vLayout->addWidget(p.controls);
            layout->addLayout(vLayout);
            setLayout(layout);
        }
        
        TimelineWidget::~TimelineWidget()
        {}

        void TimelineWidget::setTimeObject(qt::TimeObject* timeObject)
        {
            TLR_PRIVATE_P();
            p.slider->setTimeObject(timeObject);
            p.controls->setTimeObject(timeObject);
        }

        void TimelineWidget::setColorConfig(const gl::ColorConfig& colorConfig)
        {
            TLR_PRIVATE_P();
            p.viewport->setColorConfig(colorConfig);
            p.slider->setColorConfig(colorConfig);
        }

        void TimelineWidget::setImageOptions(const gl::ImageOptions& imageOptions)
        {
            TLR_PRIVATE_P();
            p.viewport->setImageOptions(imageOptions);
        }

        void TimelineWidget::setTimelinePlayer(qt::TimelinePlayer* timelinePlayer)
        {
            TLR_PRIVATE_P();
            p.viewport->setTimelinePlayer(timelinePlayer);
            p.slider->setTimelinePlayer(timelinePlayer);
            p.controls->setTimelinePlayer(timelinePlayer);
        }
    }
}
