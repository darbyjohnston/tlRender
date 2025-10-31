// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#pragma once

#include <ftk/UI/Window.h>

namespace tl
{
    namespace timelineui
    {
        //! Window.
        //! 
        //! This window creates a timeline renderer for use by the other
        //! timeline widgets.
        class Window : public ftk::Window
        {
            FTK_NON_COPYABLE(Window);

        protected:
            void _init(
                const std::shared_ptr<ftk::Context>&,
                const std::string& name,
                const ftk::Size2I&);

            Window() = default;

        public:
            virtual ~Window();

            //! Create a new window.
            static std::shared_ptr<Window> create(
                const std::shared_ptr<ftk::Context>&,
                const std::string& name,
                const ftk::Size2I&);

        protected:
            std::shared_ptr<ftk::IRender> _createRender(const std::shared_ptr<ftk::LogSystem>&) override;
        };
    }
}
