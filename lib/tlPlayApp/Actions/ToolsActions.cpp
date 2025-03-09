// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlPlayApp/Actions/ToolsActions.h>

#include <tlPlayApp/Models/ToolsModel.h>
#include <tlPlayApp/App.h>

namespace tl
{
    namespace play
    {
        void ToolsActions::_init(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app)
        {
            IActions::_init(context, app, "Tools");

            auto appWeak = std::weak_ptr<App>(app);
            const auto enums = getToolEnums();
            const auto labels = getToolLabels();
            for (size_t i = 0; i < enums.size(); ++i)
            {
                const auto tool = enums[i];
                auto action = dtk::Action::create(
                    getText(tool),
                    getIcon(tool),
                    [appWeak, tool](bool value)
                    {
                        if (auto app = appWeak.lock())
                        {
                            auto toolsModel = app->getToolsModel();
                            const Tool active = toolsModel->getActiveTool();
                            toolsModel->setActiveTool(tool != active ? tool : Tool::None);
                        }
                    });
                _actions[labels[i]] = action;
            }

            _tooltips =
            {
                { "Files", "Toggle the files tool." },
                { "Export", "Toggle the export tool." },
                { "View", "Toggle the view tool." },
                { "ColorPicker", "Toggle the color picker tool." },
                { "ColorControls", "Toggle the color controls tool." },
                { "Info", "Toggle the information tool." },
                { "Audio", "Toggle the audio tool." },
                { "Devices", "Toggle the devices tool." },
                { "Settings", "Toggle the settings." },
                { "Messages", "Toggle the messages." },
                { "SystemLog", "Toggle the system log." }
            };

            _keyShortcutsUpdate(app->getSettingsModel()->getKeyShortcuts());
        }

        ToolsActions::~ToolsActions()
        {}

        std::shared_ptr<ToolsActions> ToolsActions::create(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app)
        {
            auto out = std::shared_ptr<ToolsActions>(new ToolsActions);
            out->_init(context, app);
            return out;
        }
    }
}
