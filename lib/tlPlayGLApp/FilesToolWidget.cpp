// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlPlayGLApp/FilesToolWidget.h>

#include <tlPlayGLApp/App.h>

namespace tl
{
    namespace play_gl
    {
        struct FilesToolWidget::Private
        {};

        void FilesToolWidget::_init(
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IToolWidget::_init(
                Tool::Files,
                "tl::play_gl::FilesToolWidget",
                app,
                context,
                parent);
        }

        FilesToolWidget::FilesToolWidget() :
            _p(new Private)
        {}

        FilesToolWidget::~FilesToolWidget()
        {}

        std::shared_ptr<FilesToolWidget> FilesToolWidget::create(
            const std::shared_ptr<App>&app,
            const std::shared_ptr<system::Context>&context,
            const std::shared_ptr<IWidget>&parent)
        {
            auto out = std::shared_ptr<FilesToolWidget>(new FilesToolWidget);
            out->_init(app, context, parent);
            return out;
        }
    }
}
