// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include "FileMenu.h"

#include "App.h"

namespace tl
{
    namespace examples
    {
        namespace play_glfw
        {
            struct FileMenu::Private
            {
                std::shared_ptr<Menu> recentMenu;
                std::shared_ptr<Menu> currentMenu;
            };

            void FileMenu::_init(
                const std::shared_ptr<App>& app,
                const std::shared_ptr<system::Context>& context)
            {
                Menu::_init(context);
                TLRENDER_P();

                auto openAction = ui::Action::create(context);
                openAction->setText("Open");
                openAction->setShortut(
                    ui::Key::O,
                    static_cast<int>(ui::KeyModifier::Control));
                openAction->setIcon("FileOpen");
                openAction->setClickedCallback(
                    [this]
                    {
                        close();
                    });
                addAction(openAction);

                auto openWithAudioAction = ui::Action::create(context);
                openWithAudioAction->setText("Open With Separate Audio");
                openWithAudioAction->setShortut(
                    ui::Key::O,
                    static_cast<int>(ui::KeyModifier::Shift) |
                    static_cast<int>(ui::KeyModifier::Control));
                openWithAudioAction->setIcon("FileOpenSeparateAudio");
                openWithAudioAction->setClickedCallback(
                    [this]
                    {
                        close();
                    });
                addAction(openWithAudioAction);

                auto closeAction = ui::Action::create(context);
                closeAction->setText("Close");
                closeAction->setShortut(
                    ui::Key::E,
                    static_cast<int>(ui::KeyModifier::Control));
                closeAction->setIcon("FileClose");
                closeAction->setClickedCallback(
                    [this]
                    {
                        close();
                    });
                addAction(closeAction);

                auto closeAllAction = ui::Action::create(context);
                closeAllAction->setText("Close All");
                closeAllAction->setShortut(
                    ui::Key::E,
                    static_cast<int>(ui::KeyModifier::Shift) |
                    static_cast<int>(ui::KeyModifier::Control));
                closeAllAction->setIcon("FileCloseAll");
                closeAllAction->setClickedCallback(
                    [this]
                    {
                        close();
                    });
                addAction(closeAllAction);

                auto reloadAction = ui::Action::create(context);
                reloadAction->setText("Reload");
                reloadAction->setClickedCallback(
                    [this]
                    {
                        close();
                    });
                addAction(reloadAction);

                p.recentMenu = addSubMenu("Recent");
                for (size_t i = 0; i < 10; ++i)
                {
                    auto recentAction = ui::Action::create(context);
                    recentAction->setText("File name");
                    recentAction->setClickedCallback(
                        [this]
                        {
                            close();
                        });
                    p.recentMenu->addAction(recentAction);
                }

                addDivider();

                p.currentMenu = addSubMenu("Current");
                for (size_t i = 0; i < 10; ++i)
                {
                    auto currentAction = ui::Action::create(context);
                    currentAction->setText("File name");
                    currentAction->setClickedCallback(
                        [this]
                        {
                            close();
                        });
                    p.currentMenu->addAction(currentAction);
                }

                auto nextAction = ui::Action::create(context);
                nextAction->setText("Next");
                nextAction->setShortut(
                    ui::Key::PageDown,
                    static_cast<int>(ui::KeyModifier::Control));
                nextAction->setIcon("Next");
                nextAction->setClickedCallback(
                    [this]
                    {
                        close();
                    });
                addAction(nextAction);

                auto prevAction = ui::Action::create(context);
                prevAction->setText("Prev");
                prevAction->setShortut(
                    ui::Key::PageUp,
                    static_cast<int>(ui::KeyModifier::Control));
                prevAction->setIcon("Prev");
                prevAction->setClickedCallback(
                    [this]
                    {
                        close();
                    });
                addAction(prevAction);

                addDivider();

                auto nextLayerAction = ui::Action::create(context);
                nextLayerAction->setText("Next Layer");
                nextLayerAction->setShortut(
                    ui::Key::Equal,
                    static_cast<int>(ui::KeyModifier::Control));
                nextLayerAction->setClickedCallback(
                    [this]
                    {
                        close();
                    });
                addAction(nextLayerAction);

                auto prevLayerAction = ui::Action::create(context);
                prevLayerAction->setText("Previous Layer");
                prevLayerAction->setShortut(
                    ui::Key::Minus,
                    static_cast<int>(ui::KeyModifier::Control));
                prevLayerAction->setClickedCallback(
                    [this]
                    {
                        close();
                    });
                addAction(prevLayerAction);

                addDivider();

                auto exitAction = ui::Action::create(context);
                exitAction->setText("Exit");
                exitAction->setShortut(
                    ui::Key::Q,
                    static_cast<int>(ui::KeyModifier::Control));
                exitAction->setClickedCallback(
                    [app]
                    {
                        app->exit();
                    });
                addAction(exitAction);
            }

            FileMenu::FileMenu() :
                _p(new Private)
            {}

            FileMenu::~FileMenu()
            {}

            std::shared_ptr<FileMenu> FileMenu::create(
                const std::shared_ptr<App>& app,
                const std::shared_ptr<system::Context>& context)
            {
                auto out = std::shared_ptr<FileMenu>(new FileMenu);
                out->_init(app, context);
                return out;
            }

            void FileMenu::close()
            {
                Menu::close();
                _p->recentMenu->close();
            }
        }
    }
}
