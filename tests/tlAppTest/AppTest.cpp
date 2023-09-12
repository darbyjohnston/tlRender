// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlAppTest/AppTest.h>

#include <tlApp/IApp.h>

#include <cstring>

using namespace tl::app;

namespace tl
{
    namespace app_tests
    {
        AppTest::AppTest(const std::shared_ptr<system::Context>& context) :
            ITest("AppTest::AppTest", context)
        {}

        std::shared_ptr<AppTest> AppTest::create(const std::shared_ptr<system::Context>& context)
        {
            return std::shared_ptr<AppTest>(new AppTest(context));
        }
        
        namespace
        {
            class App : public IApp
            {
                void _init(
                    const std::vector<std::string>& args,
                    const std::shared_ptr<system::Context>& context)
                {
                    auto inputArg = CmdLineValueArg<std::string>::create(
                        _input,
                        "input",
                        "This is help for the input argument.");
                    auto outputArg = CmdLineValueArg<std::string>::create(
                        _output,
                        "output",
                        "This is help for the output argument.",
                        true);
                    auto option = CmdLineValueOption<int>::create(
                        _option,
                        { "-option" },
                        "This is the help for the option.");
                    IApp::_init(
                        args,
                        context,
                        "test",
                        "Test application.",
                        { inputArg, outputArg },
                        { option });
                    
                    _log("Log test");
                    
                    _printError("Error test");
                }

            public:
                static std::shared_ptr<App> create(
                    const std::vector<std::string>& args,
                    const std::shared_ptr<system::Context>& context)
                {
                    auto out = std::shared_ptr<App>(new App);
                    out->_init(args, context);
                    return out;
                }
                
            private:
                std::string _input;
                std::string _output;
                int _option = 0;
            };
        }

        void AppTest::run()
        {
            {
                auto app = App::create({ "app" }, _context);
                TLRENDER_ASSERT(app->getContext());
                TLRENDER_ASSERT(1 == app->getExit());
            }
            {
                auto app = App::create({ "app", "-h" }, _context);
                TLRENDER_ASSERT(1 == app->getExit());
            }
            {
                auto app = App::create({ "app", "input", "-log" }, _context);
                for (size_t i = 0; i < 10; ++i)
                {
                    _context->tick();
                }
                TLRENDER_ASSERT(0 == app->getExit());
            }
            try
            {
                auto app = App::create({ "app", "input", "output", "-option" }, _context);
                TLRENDER_ASSERT(false);
            }
            catch (const std::exception&)
            {}
        }
    }
}
