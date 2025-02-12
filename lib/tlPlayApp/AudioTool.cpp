// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlPlayApp/AudioTool.h>

#include <tlPlayApp/App.h>

#include <tlPlay/AudioModel.h>

#include <dtk/ui/Bellows.h>
#include <dtk/ui/DoubleEditSlider.h>
#include <dtk/ui/RowLayout.h>
#include <dtk/ui/ScrollWidget.h>

namespace tl
{
    namespace play_app
    {
        struct AudioTool::Private
        {
            std::shared_ptr<dtk::DoubleEditSlider> syncOffsetSlider;

            std::shared_ptr<dtk::ValueObserver<double> > syncOffsetObserver;
        };

        void AudioTool::_init(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<IWidget>& parent)
        {
            IToolWidget::_init(
                context,
                app,
                Tool::Audio,
                "tl::play_app::AudioTool",
                parent);
            DTK_P();

            p.syncOffsetSlider = dtk::DoubleEditSlider::create(context);
            p.syncOffsetSlider->setRange(dtk::RangeD(-1.0, 1.0));
            p.syncOffsetSlider->setDefaultValue(0.0);

            auto layout = dtk::VerticalLayout::create(context);
            auto vLayout = dtk::VerticalLayout::create(context);
            vLayout->setMarginRole(dtk::SizeRole::MarginSmall);
            vLayout->setSpacingRole(dtk::SizeRole::SpacingSmall);
            p.syncOffsetSlider->setParent(vLayout);
            auto bellows = dtk::Bellows::create(context, "Sync Offset", layout);
            bellows->setWidget(vLayout);
            auto scrollWidget = dtk::ScrollWidget::create(context);
            scrollWidget->setBorder(false);
            scrollWidget->setWidget(layout);
            _setWidget(scrollWidget);

            auto appWeak = std::weak_ptr<App>(app);
            p.syncOffsetSlider->setCallback(
                [appWeak](double value)
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getAudioModel()->setSyncOffset(value);
                    }
                });

            p.syncOffsetObserver = dtk::ValueObserver<double>::create(
                app->getAudioModel()->observeSyncOffset(),
                [this](double value)
                {
                    _p->syncOffsetSlider->setValue(value);
                });
        }

        AudioTool::AudioTool() :
            _p(new Private)
        {}

        AudioTool::~AudioTool()
        {}

        std::shared_ptr<AudioTool> AudioTool::create(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<AudioTool>(new AudioTool);
            out->_init(context, app, parent);
            return out;
        }
    }
}
