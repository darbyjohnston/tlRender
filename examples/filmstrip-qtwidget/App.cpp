// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include "App.h"

#include "MainWindow.h"

#include <tlCore/Math.h>
#include <tlCore/StringFormat.h>
#include <tlCore/Time.h>

namespace tl
{
    namespace examples
    {
        namespace filmstrip_qtwidget
        {
            App::App(
                int& argc,
                char** argv,
                const std::shared_ptr<system::Context>& context) :
                QApplication(argc, argv)
            {
                IApp::_init(
                    argc,
                    argv,
                    context,
                    "filmstrip-qwidget",
                    "View a timeline as a series of thumbnail images.",
                    {
                        app::CmdLineValueArg<std::string>::create(
                            _input,
                            "input",
                            "The input timeline.",
                            true)
                    });
                const int exitCode = getExit();
                if (exitCode != 0)
                {
                    exit(exitCode);
                    return;
                }

                // Initialize Qt.
                QCoreApplication::setOrganizationName("tlRender");
                QCoreApplication::setApplicationName("filmstrip-qwidget");
                setStyle("Fusion");

                // Create the context object.
                _contextObject = new qt::ContextObject(context, this);

                // Create the main window.
                auto mainWindow = new MainWindow(_input, _context);
                mainWindow->show();
            }
        }
    }
}
