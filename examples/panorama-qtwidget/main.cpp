// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include "PanoramaTimelineViewport.h"

#include <tlQtWidget/Util.h>

#include <tlQt/ContextObject.h>
#include <tlQt/TimelinePlayer.h>

#include <QApplication>

#include <iostream>

int main(int argc, char* argv[])
{
    // Initialize.
    auto context = tl::system::Context::create();
    tl::qtwidget::init(context);

    // Parse the command line.
    if (argc != 2)
    {
        std::cout << "Usage: panorama-qwidget (timeline)" << std::endl;
        return 1;
    }

    // Create the Qt application.
    QApplication app(argc, argv);

    // Create the context object.
    auto contextObject = new tl::qt::ContextObject(context);

    // Create the timeline player.
    auto timeline = tl::timeline::Timeline::create(argv[1], context);
    auto timelinePlayer = new tl::qt::TimelinePlayer(tl::timeline::TimelinePlayer::create(timeline, context), context);

    // Hook up logging.
    /*std::shared_ptr<tl::observer::ValueObserver<tl::log::Item> > logObserver;
    for (const auto& i : context->getLogInit())
    {
        std::cout << "[LOG] " << tl::toString(i) << std::endl;
    }
    logObserver = tl::observer::ValueObserver<tl::log::Item>::create(
        context->getSystem<tl::log::System>()->observeLog(),
        [](const tl::log::Item & value)
        {
            std::cout << "[LOG] " << tl::toString(value) << std::endl;
        },
        tl::observer::CallbackAction::Suppress);*/

    // Create the panorama timeline viewport.
    auto timelineViewport = new tl::examples::panorama_qtwidget::PanoramaTimelineViewport(context);
    timelineViewport->setTimelinePlayer(timelinePlayer);
    timelineViewport->show();

    // Start playback.
    timelinePlayer->setPlayback(tl::timeline::Playback::Forward);

    // Start the application.
    int r = app.exec();

    return r;
}
