// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlBakeApp/App.h>

#include <tlTimeline/Util.h>

#include <iostream>

int main(int argc, char* argv[])
{
    int r = 0;
    try
    {
        auto context = tl::system::Context::create();
        tl::timeline::init(context);
        auto app = tl::bake::App::create(argc, argv, context);
        app->run();
        r = app->getExit();
    }
    catch(const std::exception& e)
    {
        std::cerr << "ERROR: " << e.what() << std::endl;
    }
    return r;
}
