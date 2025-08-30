// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <feather-tk/ui/RowLayout.h>
#include <feather-tk/ui/ToolBar.h>

namespace tl
{
    namespace play
    {
        class App;
        class CompareActions;
        class FileActions;
        class ViewActions;
        class WindowActions;

        //! File tool bar.
        class FileToolBar : public ftk::ToolBar
        {
            FTK_NON_COPYABLE(FileToolBar);

        protected:
            void _init(
                const std::shared_ptr<ftk::Context>&,
                const std::shared_ptr<FileActions>&,
                const std::shared_ptr<IWidget>& parent);

            FileToolBar() = default;

        public:
            ~FileToolBar();

            static std::shared_ptr<FileToolBar> create(
                const std::shared_ptr<ftk::Context>&,
                const std::shared_ptr<FileActions>&,
                const std::shared_ptr<IWidget>& parent = nullptr);
        };

        //! Compare tool bar.
        class CompareToolBar : public ftk::ToolBar
        {
            FTK_NON_COPYABLE(CompareToolBar);

        protected:
            void _init(
                const std::shared_ptr<ftk::Context>&,
                const std::shared_ptr<CompareActions>&,
                const std::shared_ptr<IWidget>& parent);

            CompareToolBar() = default;

        public:
            ~CompareToolBar();

            static std::shared_ptr<CompareToolBar> create(
                const std::shared_ptr<ftk::Context>&,
                const std::shared_ptr<CompareActions>&,
                const std::shared_ptr<IWidget>& parent = nullptr);
        };

        //! View tool bar.
        class ViewToolBar : public ftk::ToolBar
        {
            FTK_NON_COPYABLE(ViewToolBar);

        protected:
            void _init(
                const std::shared_ptr<ftk::Context>&,
                const std::shared_ptr<ViewActions>&,
                const std::shared_ptr<IWidget>& parent);

            ViewToolBar() = default;

        public:
            ~ViewToolBar();

            static std::shared_ptr<ViewToolBar> create(
                const std::shared_ptr<ftk::Context>&,
                const std::shared_ptr<ViewActions>&,
                const std::shared_ptr<IWidget>& parent = nullptr);
        };

        //! Window tool bar.
        class WindowToolBar : public ftk::ToolBar
        {
            FTK_NON_COPYABLE(WindowToolBar);

        protected:
            void _init(
                const std::shared_ptr<ftk::Context>&,
                const std::shared_ptr<WindowActions>&,
                const std::shared_ptr<IWidget>& parent);

            WindowToolBar() = default;

        public:
            ~WindowToolBar();

            static std::shared_ptr<WindowToolBar> create(
                const std::shared_ptr<ftk::Context>&,
                const std::shared_ptr<WindowActions>&,
                const std::shared_ptr<IWidget>& parent = nullptr);
        };

        //! Tool bars.
        class ToolBars : public ftk::IWidget
        {
            FTK_NON_COPYABLE(ToolBars);

        protected:
            void _init(
                const std::shared_ptr<ftk::Context>&,
                const std::shared_ptr<FileActions>&,
                const std::shared_ptr<CompareActions>&,
                const std::shared_ptr<ViewActions>&,
                const std::shared_ptr<WindowActions>&,
                const std::shared_ptr<IWidget>& parent);

            ToolBars() = default;

        public:
            ~ToolBars();

            static std::shared_ptr<ToolBars> create(
                const std::shared_ptr<ftk::Context>&,
                const std::shared_ptr<FileActions>&,
                const std::shared_ptr<CompareActions>&,
                const std::shared_ptr<ViewActions>&,
                const std::shared_ptr<WindowActions>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            void setGeometry(const ftk::Box2I&) override;
            void sizeHintEvent(const ftk::SizeHintEvent&) override;

        private:
            std::shared_ptr<ftk::HorizontalLayout> _layout;
        };
    }
}