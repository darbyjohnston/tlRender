// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlApp/IApp.h>

#include <tlGL/OffscreenBuffer.h>

#include <tlTimeline/IRender.h>
#include <tlTimeline/Timeline.h>

#include <tlIO/IO.h>

struct GLFWwindow;

namespace tl
{
    //! Resource application.
    namespace resource
    {
        //! Application options.
        struct Options
        {
        };

        //! Application.
        class App : public app::IApp
        {
            TLRENDER_NON_COPYABLE(App);

        protected:
            void _init(
                int argc,
                char* argv[],
                const std::shared_ptr<system::Context>&);

            App();

        public:
            ~App();

            //! Create a new application.
            static std::shared_ptr<App> create(
                int argc,
                char* argv[],
                const std::shared_ptr<system::Context>&);

            //! Run the application.
            void run();

        private:
            std::string _input;
            std::string _output;
            std::string _varName;
            Options _options;

            std::chrono::steady_clock::time_point _startTime;
        };
    }
}
