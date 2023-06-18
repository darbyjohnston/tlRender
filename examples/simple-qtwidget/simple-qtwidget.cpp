// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlQtWidget/Init.h>
#include <tlQtWidget/TimelineViewport.h>

#include <tlQt/ContextObject.h>
#include <tlQt/TimelinePlayer.h>

#include <QApplication>

#include <iostream>

int main(int argc, char* argv[])
{
    // Initialize.
    auto context = tl::system::Context::create();
    tl::qtwidget::init(context);
#if (QT_VERSION < QT_VERSION_CHECK(6, 5, 0))
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif

    // Parse the command line.
    if (argc != 2)
    {
        std::cout << "Usage: simple-qwidget (timeline)" << std::endl;
        return 1;
    }

    int r = 0;
    try
    {
        // Create the Qt application.
        QApplication app(argc, argv);

        // Create the context object.
        auto contextObject = new tl::qt::ContextObject(context);

        // Create the timeline player.
        auto timeline = tl::timeline::Timeline::create(argv[1], context);
        auto timelinePlayer = new tl::qt::TimelinePlayer(
            tl::timeline::Player::create(timeline, context), context);

        // Create the timeline viewport.
        auto timelineViewport = new tl::qtwidget::TimelineViewport(context);
        timelineViewport->setTimelinePlayers({ timelinePlayer });
        timelineViewport->show();

        // Start playback.
        timelinePlayer->setPlayback(tl::timeline::Playback::Forward);

        // Start the application.
        r = app.exec();
    }
    catch (const std::exception& e)
    {
        std::cerr << "ERROR: " << e.what() << std::endl;
        r = 1;
    }
    return r;
}
