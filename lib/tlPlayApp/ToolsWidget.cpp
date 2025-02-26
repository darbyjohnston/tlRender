// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlPlayApp/ToolsWidget.h>

#include <tlPlayApp/App.h>
#include <tlPlayApp/AudioTool.h>
#include <tlPlayApp/ColorControlsTool.h>
#include <tlPlayApp/ColorPickerTool.h>
#include <tlPlayApp/DevicesTool.h>
#include <tlPlayApp/ExportTool.h>
#include <tlPlayApp/FilesTool.h>
#include <tlPlayApp/InfoTool.h>
#include <tlPlayApp/MessagesTool.h>
#include <tlPlayApp/SettingsTool.h>
#include <tlPlayApp/SystemLogTool.h>
#include <tlPlayApp/ViewTool.h>

#include <dtk/ui/RowLayout.h>
#include <dtk/ui/StackLayout.h>

namespace tl
{
    namespace play_app
    {
        struct ToolsWidget::Private
        {
            std::map<Tool, std::shared_ptr<IToolWidget> > toolWidgets;
            std::shared_ptr<dtk::StackLayout> layout;
            std::shared_ptr<dtk::ValueObserver<Tool> > activeObserver;
        };

        void ToolsWidget::_init(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<MainWindow>& mainWindow,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init(
                context,
                "tl::play_app::ToolsWidget",
                parent);
            DTK_P();

            p.toolWidgets[Tool::Audio] = AudioTool::create(context, app);
            p.toolWidgets[Tool::ColorPicker] = ColorPickerTool::create(context, app);
            p.toolWidgets[Tool::ColorControls] = ColorControlsTool::create(context, app);
            p.toolWidgets[Tool::Devices] = DevicesTool::create(context, app);
            p.toolWidgets[Tool::Export] = ExportTool::create(context, app);
            p.toolWidgets[Tool::Files] = FilesTool::create(context, app);
            p.toolWidgets[Tool::Info] = InfoTool::create(context, app);
            p.toolWidgets[Tool::Messages] = MessagesTool::create(context, app);
            p.toolWidgets[Tool::Settings] = SettingsTool::create(context, app);
            p.toolWidgets[Tool::SystemLog] = SystemLogTool::create(context, app);
            p.toolWidgets[Tool::View] = ViewTool::create(context, app);

            p.layout = dtk::StackLayout::create(context, shared_from_this());
            for (const auto& widget : p.toolWidgets)
            {
                widget.second->setParent(p.layout);
            }

            p.activeObserver = dtk::ValueObserver<Tool>::create(
                app->getToolsModel()->observeActiveTool(),
                [this](Tool value)
                {
                    DTK_P();
                    auto i = p.toolWidgets.find(value);
                    p.layout->setCurrentWidget(i != p.toolWidgets.end() ? i->second : nullptr);
                    setVisible(value != Tool::None);
                });
        }

        ToolsWidget::ToolsWidget() :
            _p(new Private)
        {}

        ToolsWidget::~ToolsWidget()
        {}

        std::shared_ptr<ToolsWidget> ToolsWidget::create(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<MainWindow>& mainWindow,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<ToolsWidget>(new ToolsWidget);
            out->_init(context, app, mainWindow, parent);
            return out;
        }

        void ToolsWidget::setGeometry(const dtk::Box2I & value)
        {
            IWidget::setGeometry(value);
            _p->layout->setGeometry(value);
        }

        void ToolsWidget::sizeHintEvent(const dtk::SizeHintEvent & event)
        {
            IWidget::sizeHintEvent(event);
            _setSizeHint(_p->layout->getSizeHint());
        }
    }
}
