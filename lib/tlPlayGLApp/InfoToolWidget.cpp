// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlPlayGLApp/InfoToolWidget.h>

#include <tlPlayGLApp/App.h>

namespace tl
{
    namespace play_gl
    {
        struct InfoToolWidget::Private
        {};

        void InfoToolWidget::_init(
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IToolWidget::_init(
                Tool::Info,
                "tl::play_gl::InfoToolWidget",
                app,
                context,
                parent);
        }

        InfoToolWidget::InfoToolWidget() :
            _p(new Private)
        {}

        InfoToolWidget::~InfoToolWidget()
        {}

        std::shared_ptr<InfoToolWidget> InfoToolWidget::create(
            const std::shared_ptr<App>&app,
            const std::shared_ptr<system::Context>&context,
            const std::shared_ptr<IWidget>&parent)
        {
            auto out = std::shared_ptr<InfoToolWidget>(new InfoToolWidget);
            out->_init(app, context, parent);
            return out;
        }
    }
}
