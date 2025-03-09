// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlPlayApp/Actions/WindowActions.h>

#include <tlPlayApp/App.h>
#include <tlPlayApp/MainWindow.h>

#include <dtk/core/Format.h>

namespace tl
{
    namespace play
    {
        struct WindowActions::Private
        {
            std::shared_ptr<SettingsModel> model;
            std::map<std::string, std::shared_ptr<dtk::Action> > actions;

            std::shared_ptr<dtk::ValueObserver<KeyShortcutsSettings> > keyShortcutsSettingsObserver;
        };

        void WindowActions::_init(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<MainWindow>& mainWindow)
        {
            DTK_P();

            p.model = app->getSettingsModel();

            auto appWeak = std::weak_ptr<App>(app);
            p.actions["FullScreen"] = dtk::Action::create(
                "Full Screen",
                "WindowFullScreen",
                [appWeak](bool value)
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getMainWindow()->setFullScreen(value);
                    }
                });

            p.actions["FloatOnTop"] = dtk::Action::create(
                "Float On Top",
                [appWeak](bool value)
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getMainWindow()->setFloatOnTop(value);
                    }
                });

            p.actions["Secondary"] = dtk::Action::create(
                "Secondary",
                "WindowSecondary",
                dtk::Key::Y,
                0,
                [appWeak](bool value)
                {
                    if (auto app = appWeak.lock())
                    {
                        app->setSecondaryWindow(value);
                    }
                });

            p.actions["FileToolBar"] = dtk::Action::create(
                "File Tool Bar",
                [this](bool value)
                {
                    DTK_P();
                    auto options = p.model->getWindow();
                    options.fileToolBar = value;
                    p.model->setWindow(options);
                });

            p.actions["CompareToolBar"] = dtk::Action::create(
                "Compare Tool Bar",
                [this](bool value)
                {
                    DTK_P();
                    auto options = p.model->getWindow();
                    options.compareToolBar = value;
                    p.model->setWindow(options);
                });

            p.actions["WindowToolBar"] = dtk::Action::create(
                "Window Tool Bar",
                [this](bool value)
                {
                    DTK_P();
                    auto options = p.model->getWindow();
                    options.windowToolBar = value;
                    p.model->setWindow(options);
                });

            p.actions["ViewToolBar"] = dtk::Action::create(
                "View Tool Bar",
                [this](bool value)
                {
                    DTK_P();
                    auto options = p.model->getWindow();
                    options.viewToolBar = value;
                    p.model->setWindow(options);
                });

            p.actions["ToolsToolBar"] = dtk::Action::create(
                "Tools Tool Bar",
                [this](bool value)
                {
                    DTK_P();
                    auto options = p.model->getWindow();
                    options.toolsToolBar = value;
                    p.model->setWindow(options);
                });

            p.actions["Timeline"] = dtk::Action::create(
                "Timeline",
                [this](bool value)
                {
                    DTK_P();
                    auto options = p.model->getWindow();
                    options.timeline = value;
                    p.model->setWindow(options);
                });

            p.actions["BottomToolBar"] = dtk::Action::create(
                "Bottom Tool Bar",
                [this](bool value)
                {
                    DTK_P();
                    auto options = p.model->getWindow();
                    options.bottomToolBar = value;
                    p.model->setWindow(options);
                });

            p.actions["StatusToolBar"] = dtk::Action::create(
                "Status Tool Bar",
                [this](bool value)
                {
                    DTK_P();
                    auto options = p.model->getWindow();
                    options.statusToolBar = value;
                    p.model->setWindow(options);
                });

            p.keyShortcutsSettingsObserver = dtk::ValueObserver<KeyShortcutsSettings>::create(
                app->getSettingsModel()->observeKeyShortcuts(),
                [this](const KeyShortcutsSettings& value)
                {
                    _keyShortcutsUpdate(value);
                });
        }

        WindowActions::WindowActions() :
            _p(new Private)
        {}

        WindowActions::~WindowActions()
        {}

        std::shared_ptr<WindowActions> WindowActions::create(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<MainWindow>& mainWindow)
        {
            auto out = std::shared_ptr<WindowActions>(new WindowActions);
            out->_init(context, app, mainWindow);
            return out;
        }

        const std::map<std::string, std::shared_ptr<dtk::Action> >& WindowActions::getActions() const
        {
            return _p->actions;
        }

        void WindowActions::_keyShortcutsUpdate(const KeyShortcutsSettings& value)
        {
            DTK_P();
            const std::map<std::string, std::string> tooltips =
            {
                {
                    "FullScreen",
                    "Toggle the window full screen.\n"
                    "\n"
                    "Shortcut: {0}"
                },
                {
                    "Secondary",
                    "Toggle the secondary window.\n"
                    "\n"
                    "Shortcut: {0}"
                },
            };
            for (const auto& i : p.actions)
            {
                auto j = value.shortcuts.find(dtk::Format("Window/{0}").arg(i.first));
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
