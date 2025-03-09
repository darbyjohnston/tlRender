// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlPlayApp/Actions/ToolsActions.h>

#include <tlPlayApp/Models/ToolsModel.h>
#include <tlPlayApp/App.h>

#include <dtk/core/Format.h>

namespace tl
{
    namespace play
    {
        struct ToolsActions::Private
        {
            std::map<std::string, std::shared_ptr<dtk::Action> > actions;

            std::shared_ptr<dtk::ValueObserver<KeyShortcutsSettings> > keyShortcutsSettingsObserver;
        };

        void ToolsActions::_init(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app)
        {
            DTK_P();

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
                p.actions[labels[i]] = action;
            }

            p.keyShortcutsSettingsObserver = dtk::ValueObserver<KeyShortcutsSettings>::create(
                app->getSettingsModel()->observeKeyShortcuts(),
                [this](const KeyShortcutsSettings& value)
                {
                    _keyShortcutsUpdate(value);
                });
        }

        ToolsActions::ToolsActions() :
            _p(new Private)
        {}

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

        const std::map<std::string, std::shared_ptr<dtk::Action> >& ToolsActions::getActions() const
        {
            return _p->actions;
        }

        void ToolsActions::_keyShortcutsUpdate(const KeyShortcutsSettings& value)
        {
            DTK_P();
            const std::map<std::string, std::string> tooltips =
            {
                {
                    "Files",
                    "Toggle the files tool.\n"
                    "\n"
                    "Shortcut: {0}"
                },
                {
                    "Export",
                    "Toggle the export tool.\n"
                    "\n"
                    "Shortcut: {0}"
                },
                {
                    "View",
                    "Toggle the view tool.\n"
                    "\n"
                    "Shortcut: {0}"
                },
                {
                    "ColorPicker",
                    "Toggle the color picker tool.\n"
                    "\n"
                    "Shortcut: {0}"
                },
                {
                    "ColorControls",
                    "Toggle the color controls tool.\n"
                    "\n"
                    "Shortcut: {0}"
                },
                {
                    "Info",
                    "Toggle the information tool.\n"
                    "\n"
                    "Shortcut: {0}"
                },
                {
                    "Audio",
                    "Toggle the audio tool.\n"
                    "\n"
                    "Shortcut: {0}"
                },
                {
                    "Devices",
                    "Toggle the devices tool.\n"
                    "\n"
                    "Shortcut: {0}"
                },
                {
                    "Settings",
                    "Toggle the settings.\n"
                    "\n"
                    "Shortcut: {0}"
                },
                {
                    "Messages",
                    "Toggle the messages.\n"
                    "\n"
                    "Shortcut: {0}"
                },
                {
                    "SystemLog",
                    "Toggle the system log.\n"
                    "\n"
                    "Shortcut: {0}"
                }
            };
            for (const auto& i : p.actions)
            {
                auto j = value.shortcuts.find(dtk::Format("Tools/{0}").arg(i.first));
                if (j != value.shortcuts.end())
                {
                    i.second->setShortcut(j->second.key);
                    i.second->setShortcutModifiers(j->second.modifiers);
                    const auto k = tooltips.find(i.first);
                    if (k != tooltips.end())
                    {
                        i.second->setTooltip(dtk::Format(k->second).
                            arg(dtk::getShortcutLabel(j->second.key, j->second.modifiers)));
                    }
                }
            }
        }
    }
}
