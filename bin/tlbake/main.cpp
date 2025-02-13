// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlBakeApp/App.h>

#include <tlTimeline/Init.h>

#include <dtk/core/Context.h>

#include <iostream>

DTK_MAIN()
{
    int r = 1;
    try
    {
        auto context = dtk::Context::create();
        tl::timeline::init(context);
        auto args = dtk::convert(argc, argv);
        auto app = tl::bake::App::create(context, args);
        r = app->getExit();
        if (0 == r)
        {
            app->run();
        }
    }
    catch(const std::exception& e)
    {
        std::cerr << "ERROR: " << e.what() << std::endl;
    }
    return r;
}
