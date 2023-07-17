// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlPlayGLApp/AudioToolWidget.h>

#include <tlPlayGLApp/App.h>

namespace tl
{
    namespace play_gl
    {
        struct AudioToolWidget::Private
        {};

        void AudioToolWidget::_init(
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IToolWidget::_init(
                Tool::Audio,
                "tl::play_gl::AudioToolWidget",
                app,
                context,
                parent);
        }

        AudioToolWidget::AudioToolWidget() :
            _p(new Private)
        {}

        AudioToolWidget::~AudioToolWidget()
        {}

        std::shared_ptr<AudioToolWidget> AudioToolWidget::create(
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<AudioToolWidget>(new AudioToolWidget);
            out->_init(app, context, parent);
            return out;
        }
    }
}
