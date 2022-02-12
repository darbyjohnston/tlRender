// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlQWidget/TimelineViewport.h>
#include <tlQWidget/Util.h>

#include <tlQt/TimelinePlayer.h>

#include <QApplication>

#include <iostream>

int main(int argc, char* argv[])
{
    // Initialize the tlQWidget library.
    tl::qwidget::init();

    // Parse the command line.
    if (argc != 2)
    {
        std::cout << "Usage: simple-qwidget (timeline)" << std::endl;
        return 1;
    }

    // Create the Qt application.
    QApplication app(argc, argv);

    // Create the context and timeline player.
    auto context = tl::core::Context::create();
    auto timeline = tl::timeline::Timeline::create(argv[1], context);
    auto timelinePlayer = new tl::qt::TimelinePlayer(tl::timeline::TimelinePlayer::create(timeline, context), context);

    // Create the timeline viewport.
    auto timelineViewport = new tl::qwidget::TimelineViewport(context);
    timelineViewport->setTimelinePlayers({ timelinePlayer });
    timelineViewport->show();

    // Start playback.
    timelinePlayer->setPlayback(tl::timeline::Playback::Forward);

    return app.exec();
}
