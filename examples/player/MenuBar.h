// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <dtk/ui/MenuBar.h>

namespace tl
{
    namespace examples
    {
        namespace player
        {
            class FileActions;
            class WindowActions;
            class ViewActions;
            class PlaybackActions;

            //! Menu bar.
            class MenuBar : public dtk::MenuBar
            {
                DTK_NON_COPYABLE(MenuBar);

            protected:
                void _init(
                    const std::shared_ptr<dtk::Context>&,
                    const std::shared_ptr<FileActions>&,
                    const std::shared_ptr<WindowActions>&,
                    const std::shared_ptr<ViewActions>&,
                    const std::shared_ptr<PlaybackActions>&,
                    const std::shared_ptr<IWidget>& parent);

                MenuBar() = default;

            public:
                ~MenuBar();

                static std::shared_ptr<MenuBar> create(
                    const std::shared_ptr<dtk::Context>&,
                    const std::shared_ptr<FileActions>&,
                    const std::shared_ptr<WindowActions>&,
                    const std::shared_ptr<ViewActions>&,
                    const std::shared_ptr<PlaybackActions>&,
                    const std::shared_ptr<IWidget>& parent = nullptr);
            };
        }
    }
}
