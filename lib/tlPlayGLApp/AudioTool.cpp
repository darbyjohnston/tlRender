// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlPlayGLApp/AudioTool.h>

#include <tlPlayGLApp/App.h>

namespace tl
{
    namespace play_gl
    {
        struct AudioTool::Private
        {};

        void AudioTool::_init(
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IToolWidget::_init(
                Tool::Audio,
                "tl::play_gl::AudioTool",
                app,
                context,
                parent);
        }

        AudioTool::AudioTool() :
            _p(new Private)
        {}

        AudioTool::~AudioTool()
        {}

        std::shared_ptr<AudioTool> AudioTool::create(
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<AudioTool>(new AudioTool);
            out->_init(app, context, parent);
            return out;
        }
    }
}
