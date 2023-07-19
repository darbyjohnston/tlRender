// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlPlayGLApp/MessagesTool.h>

#include <tlPlayGLApp/App.h>

namespace tl
{
    namespace play_gl
    {
        struct MessagesTool::Private
        {};

        void MessagesTool::_init(
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IToolWidget::_init(
                Tool::Messages,
                "tl::play_gl::MessagesTool",
                app,
                context,
                parent);
        }

        MessagesTool::MessagesTool() :
            _p(new Private)
        {}

        MessagesTool::~MessagesTool()
        {}

        std::shared_ptr<MessagesTool> MessagesTool::create(
            const std::shared_ptr<App>&app,
            const std::shared_ptr<system::Context>&context,
            const std::shared_ptr<IWidget>&parent)
        {
            auto out = std::shared_ptr<MessagesTool>(new MessagesTool);
            out->_init(app, context, parent);
            return out;
        }
    }
}
