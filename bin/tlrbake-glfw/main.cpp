// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include "App.h"

#include <iostream>

int main(int argc, char* argv[])
{
    int r = 0;
    try
    {
        auto app = tlr::App::create(argc, argv);
        app->run();
        r = app->getExit();
    }
    catch(const std::exception& e)
    {
        std::cerr << "ERROR: " << e.what() << std::endl;
    }
    return r;
}
