// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlBaseApp/BaseApp.h>

namespace tl
{
    //! tlresource application
    namespace resource
    {
        //! Application options.
        struct Options
        {
        };

        //! Application.
        class App : public app::BaseApp
        {
            TLRENDER_NON_COPYABLE(App);

        protected:
            void _init(
                const std::shared_ptr<dtk::Context>&,
                const std::vector<std::string>&);

            App();

        public:
            ~App();

            //! Create a new application.
            static std::shared_ptr<App> create(
                const std::shared_ptr<dtk::Context>&,
                const std::vector<std::string>&);

            //! Run the application.
            int run();

        private:
            std::string _input;
            std::string _output;
            std::string _varName;
            Options _options;

            std::chrono::steady_clock::time_point _startTime;
        };
    }
}
