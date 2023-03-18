// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlResourceApp/App.h>

#include <tlIO/IOSystem.h>

#include <tlCore/File.h>
#include <tlCore/String.h>
#include <tlCore/StringFormat.h>

namespace tl
{
    namespace resource
    {
        void App::_init(
            int argc,
            char* argv[],
            const std::shared_ptr<system::Context>& context)
        {
            IApp::_init(
                argc,
                argv,
                context,
                "tlresource",
                "Convert a resource file to a source code file.",
                {
                    app::CmdLineValueArg<std::string>::create(
                        _input,
                        "input",
                        "The input resource file."),
                    app::CmdLineValueArg<std::string>::create(
                        _output,
                        "output",
                        "The output source code file.")
                },
                {});
        }

        App::App()
        {}

        App::~App()
        {}

        std::shared_ptr<App> App::create(
            int argc,
            char* argv[],
            const std::shared_ptr<system::Context>& context)
        {
            auto out = std::shared_ptr<App>(new App);
            out->_init(argc, argv, context);
            return out;
        }

        void App::run()
        {
            if (_exit != 0)
            {
                return;
            }

            _startTime = std::chrono::steady_clock::now();

            const auto now = std::chrono::steady_clock::now();
            const std::chrono::duration<float> diff = now - _startTime;
            _print(string::Format("Seconds elapsed: {0}").arg(diff.count()));
        }
    }
}
