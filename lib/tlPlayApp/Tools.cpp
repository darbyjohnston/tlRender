// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlPlayApp/Tools.h>

#include <dtk/core/Error.h>
#include <dtk/core/String.h>

#include <array>
#include <iostream>
#include <sstream>

namespace tl
{
    namespace play_app
    {
        DTK_ENUM_IMPL(
            Tool,
            "Files",
            "View",
            "Color",
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
                "Files",
                "View",
                "Color",
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
                "Files",
                "View",
                "Color",
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
                dtk::Key::F1,
                dtk::Key::F2,
                dtk::Key::F3,
                dtk::Key::F4,
                dtk::Key::F5,
                dtk::Key::F6,
                dtk::Key::F7,
                dtk::Key::F8,
                dtk::Key::F9
            };
            return data[static_cast<size_t>(value)];
        }

        std::vector<Tool> toolsInToolbar()
        {
            const std::vector<Tool> out
            {
                Tool::Files,
                Tool::View,
                Tool::Color,
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
            std::shared_ptr<dtk::ObservableValue<int> > activeTool;
        };

        void ToolsModel::_init()
        {
            DTK_P();
            p.activeTool = dtk::ObservableValue<int>::create(-1);
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

        int ToolsModel::getActiveTool() const
        {
            return _p->activeTool->get();
        }

        std::shared_ptr<dtk::ObservableValue<int> > ToolsModel::observeActiveTool() const
        {
            return _p->activeTool;
        }

        void ToolsModel::setActiveTool(int value)
        {
            _p->activeTool->setIfChanged(value);
        }
    }
}
