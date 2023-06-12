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
            };

            void FileMenu::_init(
                const std::shared_ptr<App>& app,
                const std::shared_ptr<system::Context>& context)
            {
                Menu::_init(context);
                TLRENDER_P();

                auto openAction = ui::Action::create(context);
                openAction->setText("Open");
                addAction(openAction);

                auto closeAction = ui::Action::create(context);
                closeAction->setText("Close");
                addAction(closeAction);

                addDivider();

                auto exitAction = ui::Action::create(context);
                exitAction->setText("Exit");
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
        }
    }
}
