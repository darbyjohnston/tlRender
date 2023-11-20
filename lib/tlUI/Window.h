// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUI/IWidget.h>

namespace tl
{
    namespace ui
    {
        //! Window.
        class Window : public IWidget
        {
            TLRENDER_NON_COPYABLE(Window);

        protected:
            void _init(
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent);

            Window();

        public:
            virtual ~Window();

            //! Create a new window.
            static std::shared_ptr<Window> create(
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            //! Observe the window size.
            std::shared_ptr<observer::IValue<math::Size2i> > observeSize() const;

            //! Resize the window.
            void resize(const math::Size2i&);

            //! Get whether the window is in full screen mode.
            bool isFullScreen() const;

            //! Observe whether the window is in full screen mode.
            std::shared_ptr<observer::IValue<bool> > observeFullScreen() const;

            //! Set whether the window is in full screen mode.
            void setFullScreen(bool);

            //! Get whether the window is floating on top.
            bool isFloatOnTop() const;

            //! Observe whether the window is floating on top.
            std::shared_ptr<observer::IValue<bool> > observeFloatOnTop() const;

            //! Set whether the window is floating on top.
            void setFloatOnTop(bool);

            void setGeometry(const math::Box2i&) override;

        private:
            TLRENDER_PRIVATE();
        };
    }
}
