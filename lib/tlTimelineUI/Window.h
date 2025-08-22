// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <feather-tk/ui/Window.h>

namespace tl
{
    namespace timelineui
    {
        //! Window.
        //! 
        //! This window creates a timeline renderer for use by the other
        //! timeline widgets.
        class Window : public feather_tk::Window
        {
            FEATHER_TK_NON_COPYABLE(Window);

        protected:
            void _init(
                const std::shared_ptr<feather_tk::Context>&,
                const std::string& name,
                const feather_tk::Size2I&);

            Window() = default;

        public:
            virtual ~Window();

            //! Create a new window.
            static std::shared_ptr<Window> create(
                const std::shared_ptr<feather_tk::Context>&,
                const std::string& name,
                const feather_tk::Size2I&);

        protected:
            std::shared_ptr<feather_tk::IRender> _createRender(const std::shared_ptr<feather_tk::LogSystem>&) override;
        };
    }
}
