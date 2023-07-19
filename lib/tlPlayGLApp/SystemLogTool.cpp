// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlPlayGLApp/SystemLogTool.h>

#include <tlPlayGLApp/App.h>

namespace tl
{
    namespace play_gl
    {
        struct SystemLogTool::Private
        {};

        void SystemLogTool::_init(
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IToolWidget::_init(
                Tool::SystemLog,
                "tl::play_gl::SystemLogTool",
                app,
                context,
                parent);
        }

        SystemLogTool::SystemLogTool() :
            _p(new Private)
        {}

        SystemLogTool::~SystemLogTool()
        {}

        std::shared_ptr<SystemLogTool> SystemLogTool::create(
            const std::shared_ptr<App>&app,
            const std::shared_ptr<system::Context>&context,
            const std::shared_ptr<IWidget>&parent)
        {
            auto out = std::shared_ptr<SystemLogTool>(new SystemLogTool);
            out->_init(app, context, parent);
            return out;
        }
    }
}
