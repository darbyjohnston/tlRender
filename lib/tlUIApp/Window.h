// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUI/IWindow.h>

namespace dtk
{
    namespace gl
    {
        class Window;
    }
}

namespace tl
{
    namespace ui_app
    {
        //! Window.
        class Window : public ui::IWindow
        {
            DTK_NON_COPYABLE(Window);

        protected:
            void _init(
                const std::shared_ptr<dtk::Context>&,
                const std::string& name,
                const std::shared_ptr<Window>& share);

            Window();

        public:
            virtual ~Window();

            //! Create a new window.
            static std::shared_ptr<Window> create(
                const std::shared_ptr<dtk::Context>&,
                const std::string& name,
                const std::shared_ptr<Window>& share = nullptr);

            //! Get the window size.
            const dtk::Size2I& getWindowSize() const;

            //! Observe the window size.
            std::shared_ptr<dtk::IObservableValue<dtk::Size2I> > observeWindowSize() const;

            //! Set the window size.
            void setWindowSize(const dtk::Size2I&);

            //! Observe whether the window is visible.
            std::shared_ptr<dtk::IObservableValue<bool> > observeVisible() const;

            //! Get which screen the window is on.
            int getScreen() const;

            //! Get whether the window is in full screen mode.
            bool isFullScreen() const;

            //! Observe whether the window is in full screen mode.
            std::shared_ptr<dtk::IObservableValue<bool> > observeFullScreen() const;

            //! Set whether the window is in full screen mode.
            void setFullScreen(bool, int screen = -1);

            //! Get whether the window is floating on top.
            bool isFloatOnTop() const;

            //! Observe whether the window is floating on top.
            std::shared_ptr<dtk::IObservableValue<bool> > observeFloatOnTop() const;

            //! Set whether the window is floating on top.
            void setFloatOnTop(bool);

            //! Observe when the window is closed.
            std::shared_ptr<dtk::IObservableValue<bool> > observeClose() const;

            //! Get the color buffer type.
            dtk::ImageType getColorBuffer() const;

            //! Observe the color buffer type.
            std::shared_ptr<dtk::IObservableValue<dtk::ImageType> > observeColorBuffer() const;

            //! Set the color buffer type.
            void setColorBuffer(dtk::ImageType);

            //! Get the OpenGL window.
            const std::shared_ptr<dtk::gl::Window>& getGLWindow() const;

            void setGeometry(const dtk::Box2I&) override;
            void setVisible(bool) override;
            void tickEvent(
                bool parentsVisible,
                bool parentsEnabled,
                const ui::TickEvent&) override;

        protected:
            void _makeCurrent();
            void _doneCurrent();

        private:
            bool _hasSizeUpdate(const std::shared_ptr<IWidget>&) const;
            void _sizeHintEventRecursive(
                const std::shared_ptr<IWidget>&,
                const ui::SizeHintEvent&);

            bool _hasDrawUpdate(const std::shared_ptr<IWidget>&) const;
            void _drawEventRecursive(
                const std::shared_ptr<IWidget>&,
                const dtk::Box2I&,
                const ui::DrawEvent&);

            DTK_PRIVATE();
        };
    }
}
