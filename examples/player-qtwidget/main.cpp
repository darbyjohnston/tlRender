// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include "MainWindow.h"

#include <tlQtWidget/Init.h>

#include <tlQt/ContextObject.h>
#include <tlQt/PlayerObject.h>

#include <ftk/Core/Context.h>

#include <QApplication>

#include <iostream>

int main(int argc, char* argv[])
{
    // Initialize.
    auto context = ftk::Context::create();
    tl::qtwidget::init(
        context,
        tl::qt::DefaultSurfaceFormat::OpenGL_4_1_CoreProfile);
#if (QT_VERSION < QT_VERSION_CHECK(6, 5, 0))
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif

    // Parse the command line.
    if (argc != 2)
    {
        std::cout << "Usage: player-qwidget (timeline)" << std::endl;
        return 1;
    }

    int r = 1;
    try
    {
        // Create the Qt application.
        QApplication app(argc, argv);

        // Create the context object.
        QScopedPointer<tl::qt::ContextObject> contextObject(
            new tl::qt::ContextObject(context));

        // Create the timeline.
        auto timeline = tl::timeline::Timeline::create(context, argv[1]);

        // Create the timeline player.
        QSharedPointer<tl::qt::PlayerObject> player(new tl::qt::PlayerObject(
            context,
            tl::timeline::Player::create(context, timeline)));

        // Create the main window.
        auto mainWindow = new tl::examples::player_qtwidget::MainWindow(context);
        mainWindow->setPlayer(player);
        mainWindow->resize(1280, 720);
        mainWindow->show();

        // Start playback.
        player->forward();

        // Start the application.
        r = app.exec();
    }
    catch (const std::exception& e)
    {
        std::cerr << "ERROR: " << e.what() << std::endl;
    }
    return r;
}
