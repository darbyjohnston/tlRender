// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlPlayApp/Tools/Tools.h>

#include <dtk/core/Error.h>
#include <dtk/core/String.h>

#include <array>
#include <iostream>
#include <sstream>

namespace tl
{
    namespace play
    {
        DTK_ENUM_IMPL(
            Tool,
            "None",
            "Files",
            "Export",
            "View",
            "ColorPicker",
            "ColorControls",
            "Info",
            "Audio",
            "Devices",
            "Settings",
            "Messages",
            "SystemLog");

        std::string getText(Tool value)
        {
            const std::array<std::string, static_cast<size_t>(Tool::Count)> data =
            {
                "",
                "Files",
                "Export",
                "View",
                "Color Picker",
                "Color Controls",
                "Information",
                "Audio",
                "Devices",
                "Settings",
                "Messages",
                "System Log"
            };
            return data[static_cast<size_t>(value)];
        }

        std::string getIcon(Tool value)
        {
            const std::array<std::string, static_cast<size_t>(Tool::Count)> data =
            {
                "",
                "Files",
                "Export",
                "View",
                "ColorPicker",
                "ColorControls",
                "Info",
                "Audio",
                "Devices",
                "Settings",
                "Messages",
                ""
            };
            return data[static_cast<size_t>(value)];
        }

        dtk::Key getShortcut(Tool value)
        {
            const std::array<dtk::Key, static_cast<size_t>(Tool::Count)> data =
            {
                dtk::Key::Unknown,
                dtk::Key::F1,
                dtk::Key::F2,
                dtk::Key::F3,
                dtk::Key::F4,
                dtk::Key::F5,
                dtk::Key::F6,
                dtk::Key::F7,
                dtk::Key::F8,
                dtk::Key::F9,
                dtk::Key::F10,
                dtk::Key::F11
            };
            return data[static_cast<size_t>(value)];
        }

        std::string getTooltip(Tool value)
        {
            const std::array<std::string, static_cast<size_t>(Tool::Count)> data =
            {
                std::string(),
                "Show the files tool\n"
                "\n"
                "Shortcut: {0}",
                "Show the export tool\n"
                "\n"
                "Shortcut: {0}",
                "Show the view tool\n"
                "\n"
                "Shortcut: {0}",
                "Show the color picker tool\n"
                "\n"
                "Shortcut: {0}",
                "Show the color controls tool\n"
                "\n"
                "Shortcut: {0}",
                "Show the information tool\n"
                "\n"
                "Shortcut: {0}",
                "Show the audio tool\n"
                "\n"
                "Shortcut: {0}",
                "Show the devices tool\n"
                "\n"
                "Shortcut: {0}",
                "Show the settings\n"
                "\n"
                "Shortcut: {0}",
                "Show the messages\n"
                "\n"
                "Shortcut: {0}",
                "Show the system log\n"
                "\n"
                "Shortcut: {0}"
            };
            return data[static_cast<size_t>(value)];
        }

        std::vector<Tool> getToolsInToolbar()
        {
            const std::vector<Tool> out
            {
                Tool::Files,
                Tool::Export,
                Tool::View,
                Tool::ColorPicker,
                Tool::ColorControls,
                Tool::Info,
                Tool::Audio,
                Tool::Devices,
                Tool::Settings,
                Tool::Messages
            };
            return out;
        }

        struct ToolsModel::Private
        {
            std::shared_ptr<dtk::ObservableValue<Tool> > activeTool;
        };

        void ToolsModel::_init()
        {
            DTK_P();
            p.activeTool = dtk::ObservableValue<Tool>::create(Tool::None);
        }

        ToolsModel::ToolsModel() :
            _p(new Private)
        {}

        ToolsModel::~ToolsModel()
        {}

        std::shared_ptr<ToolsModel> ToolsModel::create()
        {
            auto out = std::shared_ptr<ToolsModel>(new ToolsModel);
            out->_init();
            return out;
        }

        Tool ToolsModel::getActiveTool() const
        {
            return _p->activeTool->get();
        }

        std::shared_ptr<dtk::ObservableValue<Tool> > ToolsModel::observeActiveTool() const
        {
            return _p->activeTool;
        }

        void ToolsModel::setActiveTool(Tool value)
        {
            _p->activeTool->setIfChanged(value);
        }
    }
}
