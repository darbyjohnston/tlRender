// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrQt/TimelinePlayer.h>
#include <tlrQt/TimelineViewport.h>

#include <QApplication>

#include <iostream>

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        std::cout << "Usage: simple-qwidget (timeline)" << std::endl;
        return 1;
    }
    QApplication app(argc, argv);

    // Create the context and timeline player.
    auto context = tlr::core::Context::create();
    auto timelinePlayer = new tlr::qt::TimelinePlayer(tlr::file::Path(argv[1]), context);

    // Create the timeline viewport.
    auto timelineViewport = new tlr::qt::TimelineViewport();
    timelineViewport->setTimelinePlayer(timelinePlayer);
    timelineViewport->show();

    // Play the timeline.
    timelinePlayer->setPlayback(tlr::timeline::Playback::Forward);

    return app.exec();
}
