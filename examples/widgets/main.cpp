// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include "App.h"

#include <tlUI/Init.h>

#include <iostream>

TLRENDER_MAIN()
{
    int r = 1;
    try
    {
        auto context = dtk::Context::create();
        tl::ui::init(context);
        auto app = tl::examples::widgets::App::create(
            context,
            tl::app::convert(argc, argv));
        r = app->run();
    }
    catch(const std::exception& e)
    {
        std::cerr << "ERROR: " << e.what() << std::endl;
    }
    return r;
}
