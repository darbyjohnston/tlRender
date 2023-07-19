// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlPlayGLApp/InfoTool.h>

#include <tlPlayGLApp/App.h>

namespace tl
{
    namespace play_gl
    {
        struct InfoTool::Private
        {};

        void InfoTool::_init(
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IToolWidget::_init(
                Tool::Info,
                "tl::play_gl::InfoTool",
                app,
                context,
                parent);
        }

        InfoTool::InfoTool() :
            _p(new Private)
        {}

        InfoTool::~InfoTool()
        {}

        std::shared_ptr<InfoTool> InfoTool::create(
            const std::shared_ptr<App>&app,
            const std::shared_ptr<system::Context>&context,
            const std::shared_ptr<IWidget>&parent)
        {
            auto out = std::shared_ptr<InfoTool>(new InfoTool);
            out->_init(app, context, parent);
            return out;
        }
    }
}
