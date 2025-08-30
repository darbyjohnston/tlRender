// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include "PanoramaViewport.h"

#include <tlQtWidget/Init.h>

#include <tlQt/ContextObject.h>
#include <tlQt/PlayerObject.h>

#include <feather-tk/core/Context.h>

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
        std::cout << "Usage: panorama-qwidget (timeline)" << std::endl;
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

        // Create the panorama timeline viewport.
        auto viewport = new tl::examples::panorama_qtwidget::PanoramaViewport(context);
        viewport->setPlayer(player);
        viewport->setAttribute(Qt::WA_DeleteOnClose);
        viewport->show();

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
