// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlResourceApp/App.h>

#include <dtk/core/CmdLine.h>
#include <dtk/core/FileIO.h>
#include <dtk/core/Format.h>

namespace tl
{
    namespace resource
    {
        void App::_init(
            const std::shared_ptr<dtk::Context>& context,
            std::vector<std::string>& argv)
        {
            IApp::_init(
                context,
                argv,
                "tlresource",
                "Convert a resource file to a source file.",
                {
                    dtk::CmdLineValueArg<std::string>::create(
                        _input,
                        "input",
                        "The input resource file."),
                    dtk::CmdLineValueArg<std::string>::create(
                        _output,
                        "output",
                        "The output source code file."),
                    dtk::CmdLineValueArg<std::string>::create(
                        _varName,
                        "variable name",
                        "The resource variable name.")
                },
                {});
        }

        App::App()
        {}

        App::~App()
        {}

        std::shared_ptr<App> App::create(
            const std::shared_ptr<dtk::Context>& context,
            std::vector<std::string>& argv)
        {
            auto out = std::shared_ptr<App>(new App);
            out->_init(context, argv);
            return out;
        }

        void App::run()
        {
            _startTime = std::chrono::steady_clock::now();

            auto inputIO = dtk::FileIO::create(_input, dtk::FileMode::Read);
            const size_t size = inputIO->getSize();
            std::vector<uint8_t> data;
            data.resize(size);
            inputIO->readU8(data.data(), size);

            auto outputIO = dtk::FileIO::create(_output, dtk::FileMode::Write);
            outputIO->write(dtk::Format("const std::vector<uint8_t> {0} = {\n").arg(_varName));
            const size_t columns = 15;
            for (size_t i = 0; i < size; i += columns)
            {
                outputIO->write("    ");
                for (size_t j = i; j < i + columns && j < size; ++j)
                {
                    outputIO->write(dtk::Format("{0}, ").
                        arg(static_cast<int>(data[j])));
                }
                outputIO->write("\n");
            }
            outputIO->write("};\n");

            const auto now = std::chrono::steady_clock::now();
            const std::chrono::duration<float> diff = now - _startTime;
            _print(dtk::Format("Seconds elapsed: {0}").arg(diff.count()));
        }
    }
}
