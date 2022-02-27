// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include "PanoramaTimelineViewport.h"

#include <tlQtWidget/Util.h>

#include <tlQt/TimelinePlayer.h>

#include <tlIO/IOSystem.h>

#include <QApplication>

#include <iostream>

int main(int argc, char* argv[])
{
    // Initialize the tlQtWidget library.
    tl::qt::widget::init();

    // Parse the command line.
    if (argc != 2)
    {
        std::cout << "Usage: panorama-qwidget (timeline)" << std::endl;
        return 1;
    }

    // Create the Qt application.
    QApplication app(argc, argv);

    // Create the context and timeline player.
    auto context = tl::core::system::Context::create();
    context->addSystem(tl::io::System::create(context));
    auto timeline = tl::timeline::Timeline::create(argv[1], context);
    auto timelinePlayer = new tl::qt::TimelinePlayer(tl::timeline::TimelinePlayer::create(timeline, context), context);

    // Hook up logging.
    /*std::shared_ptr<tl::core::observer::ValueObserver<tl::core::log::Item> > logObserver;
    for (const auto& i : context->getLogInit())
    {
        std::cout << "[LOG] " << tl::core::toString(i) << std::endl;
    }
    logObserver = tl::core::observer::ValueObserver<tl::core::log::Item>::create(
        context->getSystem<tl::core::log::System>()->observeLog(),
        [](const tl::core::log::Item & value)
        {
            std::cout << "[LOG] " << tl::core::toString(value) << std::endl;
        },
        tl::core::observer::CallbackAction::Suppress);*/

    // Create the panorama timeline viewport.
    auto timelineViewport = new tl::examples::panorama_qtwidget::PanoramaTimelineViewport(context);
    timelineViewport->setTimelinePlayer(timelinePlayer);
    timelineViewport->show();

    // Start playback.
    timelinePlayer->setPlayback(tl::timeline::Playback::Forward);

    return app.exec();
}
