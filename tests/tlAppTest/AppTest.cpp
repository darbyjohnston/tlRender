// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlAppTest/AppTest.h>

#include <tlApp/IApp.h>

#include <tlCore/FileInfo.h>
#include <tlCore/StringFormat.h>
#include <tlCore/Time.h>

#include <cstring>

#include <wchar.h>

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
        
        void AppTest::run()
        {
            _convert();
            _app();
        }

        void AppTest::_convert()
        {
            {
                const std::vector<std::string> s =
                {
                    "app",
                    "arg1",
                    "arg2"
                };
                char** argv = nullptr;
                argv = new char* [3];
                argv[0] = new char [4];
                argv[1] = new char [5];
                argv[2] = new char [5];
                strcpy(argv[0], "app");
                strcpy(argv[1], "arg1");
                strcpy(argv[2], "arg2");
                TLRENDER_ASSERT(s == convert(s.size(), argv));
                delete [] argv[0];
                delete [] argv[1];
                delete [] argv[2];
                delete [] argv;
            }
            {
                const std::vector<std::string> s =
                {
                    "app",
                    "arg1",
                    "arg2"
                };
                wchar_t** argv = nullptr;
                argv = new wchar_t* [3];
                argv[0] = new wchar_t [4];
                argv[1] = new wchar_t [5];
                argv[2] = new wchar_t [5];
                wcscpy(argv[0], L"app");
                wcscpy(argv[1], L"arg1");
                wcscpy(argv[2], L"arg2");
                TLRENDER_ASSERT(s == convert(s.size(), argv));
                delete [] argv[0];
                delete [] argv[1];
                delete [] argv[2];
                delete [] argv;
            }
        }

        namespace
        {
            class App : public IApp
            {
                void _init(
                    const std::vector<std::string>& args,
                    const std::shared_ptr<system::Context>& context)
                {
                    auto inputArg = CmdLineValueArg<file::Type>::create(
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
                file::Type _input = file::Type::First;
                std::string _output;
                int _option = 0;
            };
        }

        void AppTest::_app()
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
                auto app = App::create({ "app", "Directory", "-log" }, _context);
                for (size_t i = 0; i < 10; ++i)
                {
                    _context->log(
                        "AppTest::_app",
                        string::Format("Tick: {0}").arg(i));
                    _context->tick();
                    time::sleep(std::chrono::milliseconds(1000));
                }
                TLRENDER_ASSERT(0 == app->getExit());
            }
            try
            {
                auto app = App::create({ "app", "input" }, _context);
                TLRENDER_ASSERT(false);
            }
            catch (const std::exception&)
            {}
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
