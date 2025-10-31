// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <tlBakeApp/App.h>

#include <tlTimeline/Init.h>

#include <ftk/Core/Context.h>

#include <iostream>

FTK_MAIN()
{
    int r = 1;
    try
    {
        auto context = ftk::Context::create();
        tl::timeline::init(context);
        auto args = ftk::convert(argc, argv);
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
