// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <dtk/ui/RowLayout.h>
#include <dtk/ui/ToolBar.h>

namespace tl
{
    namespace examples
    {
        namespace player
        {
            class App;
            class FileActions;
            class ViewActions;
            class WindowActions;

            //! File tool bar.
            class FileToolBar : public dtk::ToolBar
            {
                DTK_NON_COPYABLE(FileToolBar);

            protected:
                void _init(
                    const std::shared_ptr<dtk::Context>&,
                    const std::shared_ptr<FileActions>&,
                    const std::shared_ptr<IWidget>& parent);

                FileToolBar() = default;

            public:
                ~FileToolBar();

                static std::shared_ptr<FileToolBar> create(
                    const std::shared_ptr<dtk::Context>&,
                    const std::shared_ptr<FileActions>&,
                    const std::shared_ptr<IWidget>& parent = nullptr);
            };

            //! Window tool bar.
            class WindowToolBar : public dtk::ToolBar
            {
                DTK_NON_COPYABLE(WindowToolBar);

            protected:
                void _init(
                    const std::shared_ptr<dtk::Context>&,
                    const std::shared_ptr<WindowActions>&,
                    const std::shared_ptr<IWidget>& parent);

                WindowToolBar() = default;

            public:
                ~WindowToolBar();

                static std::shared_ptr<WindowToolBar> create(
                    const std::shared_ptr<dtk::Context>&,
                    const std::shared_ptr<WindowActions>&,
                    const std::shared_ptr<IWidget>& parent = nullptr);
            };

            //! View tool bar.
            class ViewToolBar : public dtk::ToolBar
            {
                DTK_NON_COPYABLE(ViewToolBar);

            protected:
                void _init(
                    const std::shared_ptr<dtk::Context>&,
                    const std::shared_ptr<ViewActions>&,
                    const std::shared_ptr<IWidget>& parent);

                ViewToolBar() = default;

            public:
                ~ViewToolBar();

                static std::shared_ptr<ViewToolBar> create(
                    const std::shared_ptr<dtk::Context>&,
                    const std::shared_ptr<ViewActions>&,
                    const std::shared_ptr<IWidget>& parent = nullptr);
            };

            //! Tool bars.
            class ToolBars : public dtk::IWidget
            {
                DTK_NON_COPYABLE(ToolBars);

            protected:
                void _init(
                    const std::shared_ptr<dtk::Context>&,
                    const std::shared_ptr<FileActions>&,
                    const std::shared_ptr<WindowActions>&,
                    const std::shared_ptr<ViewActions>&,
                    const std::shared_ptr<IWidget>& parent);

                ToolBars() = default;

            public:
                ~ToolBars();

                static std::shared_ptr<ToolBars> create(
                    const std::shared_ptr<dtk::Context>&,
                    const std::shared_ptr<FileActions>&,
                    const std::shared_ptr<WindowActions>&,
                    const std::shared_ptr<ViewActions>&,
                    const std::shared_ptr<IWidget>& parent = nullptr);

                void setGeometry(const dtk::Box2I&) override;
                void sizeHintEvent(const dtk::SizeHintEvent&) override;

            private:
                std::shared_ptr<dtk::HorizontalLayout> _layout;
            };
        }
    }
}
