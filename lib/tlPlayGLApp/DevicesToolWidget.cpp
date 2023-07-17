// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlPlayGLApp/DevicesToolWidget.h>

#include <tlPlayGLApp/App.h>

namespace tl
{
    namespace play_gl
    {
        struct DevicesToolWidget::Private
        {};

        void DevicesToolWidget::_init(
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IToolWidget::_init(
                Tool::Devices,
                "tl::play_gl::DevicesToolWidget",
                app,
                context,
                parent);
        }

        DevicesToolWidget::DevicesToolWidget() :
            _p(new Private)
        {}

        DevicesToolWidget::~DevicesToolWidget()
        {}

        std::shared_ptr<DevicesToolWidget> DevicesToolWidget::create(
            const std::shared_ptr<App>&app,
            const std::shared_ptr<system::Context>&context,
            const std::shared_ptr<IWidget>&parent)
        {
            auto out = std::shared_ptr<DevicesToolWidget>(new DevicesToolWidget);
            out->_init(app, context, parent);
            return out;
        }
    }
}
