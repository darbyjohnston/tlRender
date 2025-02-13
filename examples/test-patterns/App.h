// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <dtk/core/IApp.h>

namespace dtk
{
    namespace gl
    {
        class Window;
    }
}

namespace tl
{
    namespace examples
    {
        //! Example test patterns application.
        namespace test_patterns
        {
            //! Application.
            class App : public dtk::IApp
            {
                DTK_NON_COPYABLE(App);

            protected:
                void _init(
                    const std::shared_ptr<dtk::Context>&,
                    std::vector<std::string>&);
                App();

            public:
                ~App();

                //! Create a new application.
                static std::shared_ptr<App> create(
                    const std::shared_ptr<dtk::Context>&,
                    std::vector<std::string>&);

                void run() override;

            private:
                std::shared_ptr<dtk::gl::Window> _window;
            };
        }
    }
}
