// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <ftk/Core/IApp.h>

namespace ftk
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
            class App : public ftk::IApp
            {
                FTK_NON_COPYABLE(App);

            protected:
                void _init(
                    const std::shared_ptr<ftk::Context>&,
                    std::vector<std::string>&);
                App();

            public:
                ~App();

                //! Create a new application.
                static std::shared_ptr<App> create(
                    const std::shared_ptr<ftk::Context>&,
                    std::vector<std::string>&);

                void run() override;

            private:
                std::shared_ptr<ftk::gl::Window> _window;
            };
        }
    }
}
