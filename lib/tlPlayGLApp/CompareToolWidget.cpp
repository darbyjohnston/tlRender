// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlPlayGLApp/CompareToolWidget.h>

#include <tlPlayGLApp/App.h>

namespace tl
{
    namespace play_gl
    {
        struct CompareToolWidget::Private
        {};

        void CompareToolWidget::_init(
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IToolWidget::_init(
                Tool::Compare,
                "tl::play_gl::CompareToolWidget",
                app,
                context,
                parent);
        }

        CompareToolWidget::CompareToolWidget() :
            _p(new Private)
        {}

        CompareToolWidget::~CompareToolWidget()
        {}

        std::shared_ptr<CompareToolWidget> CompareToolWidget::create(
            const std::shared_ptr<App>&app,
            const std::shared_ptr<system::Context>&context,
            const std::shared_ptr<IWidget>&parent)
        {
            auto out = std::shared_ptr<CompareToolWidget>(new CompareToolWidget);
            out->_init(app, context, parent);
            return out;
        }
    }
}
