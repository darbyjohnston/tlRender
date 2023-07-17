// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlPlayGLApp/SystemLogToolWidget.h>

#include <tlPlayGLApp/App.h>

namespace tl
{
    namespace play_gl
    {
        struct SystemLogToolWidget::Private
        {};

        void SystemLogToolWidget::_init(
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IToolWidget::_init(
                Tool::SystemLog,
                "tl::play_gl::SystemLogToolWidget",
                app,
                context,
                parent);
        }

        SystemLogToolWidget::SystemLogToolWidget() :
            _p(new Private)
        {}

        SystemLogToolWidget::~SystemLogToolWidget()
        {}

        std::shared_ptr<SystemLogToolWidget> SystemLogToolWidget::create(
            const std::shared_ptr<App>&app,
            const std::shared_ptr<system::Context>&context,
            const std::shared_ptr<IWidget>&parent)
        {
            auto out = std::shared_ptr<SystemLogToolWidget>(new SystemLogToolWidget);
            out->_init(app, context, parent);
            return out;
        }
    }
}
