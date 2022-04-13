// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include "App.h"

#include <tlQtWidget/Util.h>

#include <tlCore/Context.h>

#include <iostream>

int main(int argc, char* argv[])
{
    int r = 0;
    try
    {
        auto context = tl::system::Context::create();
        tl::qtwidget::init(context);
        tl::examples::filmstrip_qtwidget::App app(argc, argv, context);
        if (0 == app.getExit())
        {
            r = app.exec();
        }
        tl::qtwidget::shutdown();
    }
    catch(const std::exception& e)
    {
        std::cerr << "ERROR: " << e.what() << std::endl;
    }
    return r;
}
