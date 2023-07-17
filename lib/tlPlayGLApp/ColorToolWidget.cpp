// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlPlayGLApp/ColorToolWidget.h>

#include <tlPlayGLApp/App.h>

namespace tl
{
    namespace play_gl
    {
        struct ColorToolWidget::Private
        {};

        void ColorToolWidget::_init(
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IToolWidget::_init(
                Tool::Color,
                "tl::play_gl::ColorToolWidget",
                app,
                context,
                parent);
        }

        ColorToolWidget::ColorToolWidget() :
            _p(new Private)
        {}

        ColorToolWidget::~ColorToolWidget()
        {}

        std::shared_ptr<ColorToolWidget> ColorToolWidget::create(
            const std::shared_ptr<App>&app,
            const std::shared_ptr<system::Context>&context,
            const std::shared_ptr<IWidget>&parent)
        {
            auto out = std::shared_ptr<ColorToolWidget>(new ColorToolWidget);
            out->_init(app, context, parent);
            return out;
        }
    }
}
