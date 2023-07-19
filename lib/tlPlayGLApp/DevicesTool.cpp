// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlPlayGLApp/DevicesTool.h>

#include <tlPlayGLApp/App.h>

namespace tl
{
    namespace play_gl
    {
        struct DevicesTool::Private
        {};

        void DevicesTool::_init(
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IToolWidget::_init(
                Tool::Devices,
                "tl::play_gl::DevicesTool",
                app,
                context,
                parent);
        }

        DevicesTool::DevicesTool() :
            _p(new Private)
        {}

        DevicesTool::~DevicesTool()
        {}

        std::shared_ptr<DevicesTool> DevicesTool::create(
            const std::shared_ptr<App>&app,
            const std::shared_ptr<system::Context>&context,
            const std::shared_ptr<IWidget>&parent)
        {
            auto out = std::shared_ptr<DevicesTool>(new DevicesTool);
            out->_init(app, context, parent);
            return out;
        }
    }
}
