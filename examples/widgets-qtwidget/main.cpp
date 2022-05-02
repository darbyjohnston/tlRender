// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include "App.h"

#include <tlQtWidget/Util.h>

#include <tlCore/Context.h>

#include <iostream>

int main(int argc, char* argv[])
{
    auto context = tl::system::Context::create();
    tl::qtwidget::init(context);
    tl::examples::widgets_qtwidget::App app(argc, argv, context);
    int r = app.exec();
    return r;
}
