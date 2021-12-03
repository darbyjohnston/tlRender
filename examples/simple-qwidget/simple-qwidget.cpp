// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrQWidget/TimelineViewport.h>
#include <tlrQWidget/Util.h>

#include <tlrQt/TimelinePlayer.h>

#include <QApplication>

#include <iostream>

int main(int argc, char* argv[])
{
    // Initialize the tlrQWidget library.
    tlr::qwidget::init();

    // Parse the command line.
    if (argc != 2)
    {
        std::cout << "Usage: simple-qwidget (timeline)" << std::endl;
        return 1;
    }

    // Create the Qt application.
    QApplication app(argc, argv);

    // Create the context and timeline player.
    auto context = tlr::core::Context::create();
    auto timeline = tlr::timeline::Timeline::create(argv[1], context);
    auto timelinePlayer = new tlr::qt::TimelinePlayer(tlr::timeline::TimelinePlayer::create(timeline, context), context);

    // Create the timeline viewport.
    auto timelineViewport = new tlr::qwidget::TimelineViewport(context);
    timelineViewport->setTimelinePlayer(timelinePlayer);
    timelineViewport->show();

    // Start playback.
    timelinePlayer->setPlayback(tlr::timeline::Playback::Forward);

    return app.exec();
}
