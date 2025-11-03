// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <tlPlayApp/App.h>

#include <tlTimelineUI/Init.h>

#include <tlDevice/Init.h>

int main(int argc, char* argv[])
{
    int r = 1;
    try
    {
        auto context = ftk::Context::create();
        tl::timelineui::init(context);
        tl::device::init(context);
        auto args = ftk::convert(argc, argv);
        auto app = tl::play::App::create(context, args);
        r = app->getExit();
        if (0 == r)
        {
            app->run();
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << "ERROR: " << e.what() << std::endl;
    }
    return r;
}
