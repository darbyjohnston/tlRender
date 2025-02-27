// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlPlayApp/Tools/ExportTool.h>

#include <tlPlayApp/App.h>

#include <dtk/ui/RowLayout.h>
#include <dtk/ui/ScrollWidget.h>

namespace tl
{
    namespace play
    {
        struct ExportTool::Private
        {
        };

        void ExportTool::_init(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<IWidget>& parent)
        {
            IToolWidget::_init(
                context,
                app,
                Tool::Export,
                "tl::play_app::ExportTool",
                parent);
            DTK_P();
        }

        ExportTool::ExportTool() :
            _p(new Private)
        {
        }

        ExportTool::~ExportTool()
        {
        }

        std::shared_ptr<ExportTool> ExportTool::create(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<ExportTool>(new ExportTool);
            out->_init(context, app, parent);
            return out;
        }

        void ExportTool::_widgetUpdate()
        {
            DTK_P();
        }
    }
}
