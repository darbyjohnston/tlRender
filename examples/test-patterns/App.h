// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <feather-tk/core/IApp.h>

namespace feather_tk
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
            class App : public feather_tk::IApp
            {
                FEATHER_TK_NON_COPYABLE(App);

            protected:
                void _init(
                    const std::shared_ptr<feather_tk::Context>&,
                    std::vector<std::string>&);
                App();

            public:
                ~App();

                //! Create a new application.
                static std::shared_ptr<App> create(
                    const std::shared_ptr<feather_tk::Context>&,
                    std::vector<std::string>&);

                void run() override;

            private:
                std::shared_ptr<feather_tk::gl::Window> _window;
            };
        }
    }
}
