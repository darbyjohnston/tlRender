// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlQWidget/TimelineWidget.h>

#include <tlQWidget/TimelineControls.h>
#include <tlQWidget/TimelineSlider.h>
#include <tlQWidget/TimelineViewport.h>

#include <QVBoxLayout>

namespace tl
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
            TLRENDER_P();

            p.viewport = new TimelineViewport(context);

            p.slider = new TimelineSlider;
            p.slider->setToolTip(tr("Timeline slider"));

            p.controls = new TimelineControls;

            auto layout = new QVBoxLayout;
            layout->setContentsMargins(0, 0, 0, 0);
            layout->setSpacing(0);
            layout->addWidget(p.viewport, 1);
            auto vLayout = new QVBoxLayout;
            vLayout->setContentsMargins(5, 5, 5, 5);
            vLayout->setSpacing(5);
            vLayout->addWidget(p.slider, 1);
            vLayout->addWidget(p.controls);
            layout->addLayout(vLayout);
            setLayout(layout);
        }
        
        TimelineWidget::~TimelineWidget()
        {}

        TimelineViewport* TimelineWidget::viewport() const
        {
            return _p->viewport;
        }

        TimelineSlider* TimelineWidget::slider() const
        {
            return _p->slider;
        }

        void TimelineWidget::setTimeObject(qt::TimeObject* timeObject)
        {
            TLRENDER_P();
            p.slider->setTimeObject(timeObject);
            p.controls->setTimeObject(timeObject);
        }

        void TimelineWidget::setColorConfig(const imaging::ColorConfig& colorConfig)
        {
            TLRENDER_P();
            p.viewport->setColorConfig(colorConfig);
            p.slider->setColorConfig(colorConfig);
        }

        void TimelineWidget::setImageOptions(const std::vector<render::ImageOptions>& options)
        {
            TLRENDER_P();
            p.viewport->setImageOptions(options);
        }

        void TimelineWidget::setCompareOptions(const render::CompareOptions& options)
        {
            TLRENDER_P();
            p.viewport->setCompareOptions(options);
        }

        void TimelineWidget::setTimelinePlayers(const std::vector<qt::TimelinePlayer*>& timelinePlayers)
        {
            TLRENDER_P();
            p.viewport->setTimelinePlayers(timelinePlayers);
            p.slider->setTimelinePlayer(!timelinePlayers.empty() ? timelinePlayers[0] : nullptr);
            p.controls->setTimelinePlayer(!timelinePlayers.empty() ? timelinePlayers[0] : nullptr);
        }

        void TimelineWidget::focusCurrentFrame()
        {
            _p->controls->focusCurrentFrame();
        }
    }
}
