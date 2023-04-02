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
            class Args
            {
            public:
                Args(const std::vector<std::string>& args)
                {
                    argc = args.size();
                    argv = new char*[argc];
                    for (int i = 0; i < argc; ++i)
                    {
                        const size_t size = args[i].size();
                        argv[i] = new char[size + 1];
                        memcpy(argv[i], args[i].c_str(), size);
                        argv[i][size] = 0;
                    };
                }
                
                ~Args()
                {
                    for (int i = 0; i < argc; ++i)
                    {
                        delete [] argv[i];
                    }
                    delete [] argv;
                }
                
                int argc = 0;
                char** argv = nullptr;
            };
            
            class App : public IApp
            {
                void _init(
                    int argc,
                    char* argv[],
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
                        argc,
                        argv,
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
                    int argc,
                    char* argv[],
                    const std::shared_ptr<system::Context>& context)
                {
                    auto out = std::shared_ptr<App>(new App);
                    out->_init(argc, argv, context);
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
                const auto args = Args({ "app" });
                auto app = App::create(args.argc, args.argv, _context);
                TLRENDER_ASSERT(app->getContext());
                TLRENDER_ASSERT(1 == app->getExit());
            }
            {
                const auto args = Args({ "app", "-h" });
                auto app = App::create(args.argc, args.argv, _context);
                TLRENDER_ASSERT(1 == app->getExit());
            }
            {
                const auto args = Args({ "app", "input", "-log" });
                auto app = App::create(args.argc, args.argv, _context);
                for (size_t i = 0; i < 10; ++i)
                {
                    _context->tick();
                }
                TLRENDER_ASSERT(0 == app->getExit());
            }
            try
            {
                const auto args = Args({ "app", "input", "output", "-option" });
                auto app = App::create(args.argc, args.argv, _context);
                TLRENDER_ASSERT(false);
            }
            catch (const std::exception&)
            {}
        }
    }
}
