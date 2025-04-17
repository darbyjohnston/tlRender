// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <dtk/ui/ToolBar.h>

namespace tl
{
    namespace examples
    {
        namespace player
        {
            class App;
            class FileActions;
            class WindowActions;

            //! Tool bar.
            class ToolBar : public dtk::ToolBar
            {
                DTK_NON_COPYABLE(ToolBar);

            protected:
                void _init(
                    const std::shared_ptr<dtk::Context>&,
                    const std::shared_ptr<FileActions>&,
                    const std::shared_ptr<WindowActions>&,
                    const std::shared_ptr<IWidget>& parent);

                ToolBar() = default;

            public:
                ~ToolBar();

                static std::shared_ptr<ToolBar> create(
                    const std::shared_ptr<dtk::Context>&,
                    const std::shared_ptr<FileActions>&,
                    const std::shared_ptr<WindowActions>&,
                    const std::shared_ptr<IWidget>& parent = nullptr);
            };
        }
    }
}
