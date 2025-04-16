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
            //! Menu bar.
            class MenuBar : public dtk::MenuBar
            {
                DTK_NON_COPYABLE(MenuBar);

            protected:
                void _init(
                    const std::shared_ptr<dtk::Context>&,
                    const std::map<std::string, std::shared_ptr<dtk::Action> >& fileActions,
                    const std::map<std::string, std::shared_ptr<dtk::Action> >& playbackActions,
                    const std::map<std::string, std::shared_ptr<dtk::Action> >& viewActions,
                    const std::map<std::string, std::shared_ptr<dtk::Action> >& windowActions,
                    const std::shared_ptr<IWidget>& parent);

                MenuBar() = default;

            public:
                ~MenuBar();

                static std::shared_ptr<MenuBar> create(
                    const std::shared_ptr<dtk::Context>&,
                    const std::map<std::string, std::shared_ptr<dtk::Action> >& fileActions,
                    const std::map<std::string, std::shared_ptr<dtk::Action> >& playbackActions,
                    const std::map<std::string, std::shared_ptr<dtk::Action> >& viewActions,
                    const std::map<std::string, std::shared_ptr<dtk::Action> >& windowActions,
                    const std::shared_ptr<IWidget>& parent = nullptr);
            };
        }
    }
}
