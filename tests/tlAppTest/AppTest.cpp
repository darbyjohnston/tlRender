// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlAppTest/AppTest.h>

#include <tlBaseApp/BaseApp.h>

#include <tlCore/FileInfo.h>

#include <dtk/core/Format.h>
#include <dtk/core/Time.h>

#include <cstring>

#include <wchar.h>

using namespace tl::app;

namespace tl
{
    namespace app_tests
    {
        AppTest::AppTest(const std::shared_ptr<dtk::Context>& context) :
            ITest(context, "AppTest::AppTest")
        {}

        std::shared_ptr<AppTest> AppTest::create(const std::shared_ptr<dtk::Context>& context)
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
                DTK_ASSERT(s == convert(s.size(), argv));
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
                DTK_ASSERT(s == convert(s.size(), argv));
                delete [] argv[0];
                delete [] argv[1];
                delete [] argv[2];
                delete [] argv;
            }
        }

        namespace
        {
            class App : public BaseApp
            {
                void _init(
                    const std::shared_ptr<dtk::Context>& context,
                    const std::vector<std::string>& args)
                {
                    BaseApp::_init(
                        context,
                        args,
                        "test",
                        "Test application.",
                        {
                            CmdLineValueArg<file::Type>::create(
                                _input,
                                "input",
                                "This is help for the input argument."),
                            CmdLineValueArg<std::string>::create(
                                _output,
                                "output",
                                "This is help for the output argument.",
                                true)
                        },
                        {
                            CmdLineValueOption<int>::create(
                                _intOption,
                                { "-int" },
                                "This is the help for the option."),
                            CmdLineValueOption<file::ListSort>::create(
                                _listSortOption,
                                { "-listSort", "-ls" },
                                "This is the help for the option.")
                        });
                    
                    _log("Log test");
                    
                    _printError("Error test");
                }

            public:
                static std::shared_ptr<App> create(
                    const std::shared_ptr<dtk::Context>& context,
                    const std::vector<std::string>& args)
                {
                    auto out = std::shared_ptr<App>(new App);
                    out->_init(context, args);
                    return out;
                }
                
                file::Type getInput() const { return _input; }
                const std::string& getOutput() const { return _output; }
                int getIntOption() const { return _intOption; }
                file::ListSort getListSortOption() const { return _listSortOption; }
                
            private:
                file::Type _input = file::Type::First;
                std::string _output;
                int _intOption = 0;
                file::ListSort _listSortOption = file::ListSort::First;
            };
        }

        void AppTest::_app()
        {
            {
                auto app = App::create(_context, { "app" });
                DTK_ASSERT(app->getContext());
                DTK_ASSERT(1 == app->getExit());
            }
            {
                auto app = App::create(_context, { "app", "-h" });
                DTK_ASSERT(1 == app->getExit());
            }
            {
                auto app = App::create(
                    _context,
                    {
                        "app",
                        "directory",
                        "output",
                        "-int",
                        "10",
                        "-listSort",
                        "Extension"
                     });
                DTK_ASSERT(0 == app->getExit());
                DTK_ASSERT(file::Type::Directory == app->getInput());
                DTK_ASSERT("output" == app->getOutput());
                DTK_ASSERT(10 == app->getIntOption());
                DTK_ASSERT(file::ListSort::Extension == app->getListSortOption());
            }
            {
                auto app = App::create(_context, { "app", "directory", "-log" });
                for (size_t i = 0; i < 3; ++i)
                {
                    _context->log(
                        "AppTest::_app",
                        dtk::Format("Tick: {0}").arg(i));
                    _context->tick();
                    dtk::sleep(std::chrono::milliseconds(1000));
                }
                DTK_ASSERT(0 == app->getExit());
            }
            try
            {
                auto app = App::create(_context, { "app", "input" });
                DTK_ASSERT(false);
            }
            catch (const std::exception&)
            {}
            try
            {
                auto app = App::create(_context, { "app", "input", "-int" });
                DTK_ASSERT(false);
            }
            catch (const std::exception&)
            {}
            try
            {
                auto app = App::create(_context, { "app", "input", "-listSort" });
                DTK_ASSERT(false);
            }
            catch (const std::exception&)
            {}
        }
    }
}
