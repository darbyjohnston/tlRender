// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlPlayGLApp/SettingsToolWidget.h>

#include <tlPlayGLApp/App.h>

namespace tl
{
    namespace play_gl
    {
        struct SettingsToolWidget::Private
        {};

        void SettingsToolWidget::_init(
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IToolWidget::_init(
                Tool::Settings,
                "tl::play_gl::SettingsToolWidget",
                app,
                context,
                parent);
        }

        SettingsToolWidget::SettingsToolWidget() :
            _p(new Private)
        {}

        SettingsToolWidget::~SettingsToolWidget()
        {}

        std::shared_ptr<SettingsToolWidget> SettingsToolWidget::create(
            const std::shared_ptr<App>&app,
            const std::shared_ptr<system::Context>&context,
            const std::shared_ptr<IWidget>&parent)
        {
            auto out = std::shared_ptr<SettingsToolWidget>(new SettingsToolWidget);
            out->_init(app, context, parent);
            return out;
        }
    }
}
