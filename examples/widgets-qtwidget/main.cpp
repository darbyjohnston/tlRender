// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include "App.h"

#include <tlQtWidget/Util.h>

#include <iostream>

int main(int argc, char* argv[])
{
    tl::qtwidget::init();
    tl::examples::widgets_qtwidget::App app(argc, argv);
    return app.exec();
}
