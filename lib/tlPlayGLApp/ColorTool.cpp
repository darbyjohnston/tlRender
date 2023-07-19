// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlPlayGLApp/ColorTool.h>

#include <tlPlayGLApp/App.h>

namespace tl
{
    namespace play_gl
    {
        struct ColorTool::Private
        {};

        void ColorTool::_init(
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IToolWidget::_init(
                Tool::Color,
                "tl::play_gl::ColorTool",
                app,
                context,
                parent);
        }

        ColorTool::ColorTool() :
            _p(new Private)
        {}

        ColorTool::~ColorTool()
        {}

        std::shared_ptr<ColorTool> ColorTool::create(
            const std::shared_ptr<App>&app,
            const std::shared_ptr<system::Context>&context,
            const std::shared_ptr<IWidget>&parent)
        {
            auto out = std::shared_ptr<ColorTool>(new ColorTool);
            out->_init(app, context, parent);
            return out;
        }
    }
}
