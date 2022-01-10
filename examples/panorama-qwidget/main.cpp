// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include "PanoramaTimelineViewport.h"

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
        std::cout << "Usage: panorama-qwidget (timeline)" << std::endl;
        return 1;
    }

    // Create the Qt application.
    QApplication app(argc, argv);

    // Create the context and timeline player.
    auto context = tlr::core::Context::create();
    auto timeline = tlr::timeline::Timeline::create(argv[1], context);
    auto timelinePlayer = new tlr::qt::TimelinePlayer(tlr::timeline::TimelinePlayer::create(timeline, context), context);

    // Hook up logging.
    /*std::shared_ptr<tlr::observer::ValueObserver<tlr::core::LogItem> > logObserver;
    for (const auto& i : context->getLogInit())
    {
        std::cout << "[LOG] " << tlr::core::toString(i) << std::endl;
    }
    logObserver = tlr::observer::ValueObserver<tlr::core::LogItem>::create(
        context->getSystem<tlr::core::LogSystem>()->observeLog(),
        [](const tlr::core::LogItem & value)
        {
            std::cout << "[LOG] " << tlr::core::toString(value) << std::endl;
        },
        tlr::observer::CallbackAction::Suppress);*/

    // Create the panorama timeline viewport.
    auto timelineViewport = new PanoramaTimelineViewport(context);
    timelineViewport->setTimelinePlayer(timelinePlayer);
    timelineViewport->show();

    // Start playback.
    timelinePlayer->setPlayback(tlr::timeline::Playback::Forward);

    return app.exec();
}
