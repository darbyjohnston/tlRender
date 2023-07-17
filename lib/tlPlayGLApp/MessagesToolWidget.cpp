// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlPlayGLApp/MessagesToolWidget.h>

#include <tlPlayGLApp/App.h>

namespace tl
{
    namespace play_gl
    {
        struct MessagesToolWidget::Private
        {};

        void MessagesToolWidget::_init(
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IToolWidget::_init(
                Tool::Messages,
                "tl::play_gl::MessagesToolWidget",
                app,
                context,
                parent);
        }

        MessagesToolWidget::MessagesToolWidget() :
            _p(new Private)
        {}

        MessagesToolWidget::~MessagesToolWidget()
        {}

        std::shared_ptr<MessagesToolWidget> MessagesToolWidget::create(
            const std::shared_ptr<App>&app,
            const std::shared_ptr<system::Context>&context,
            const std::shared_ptr<IWidget>&parent)
        {
            auto out = std::shared_ptr<MessagesToolWidget>(new MessagesToolWidget);
            out->_init(app, context, parent);
            return out;
        }
    }
}
