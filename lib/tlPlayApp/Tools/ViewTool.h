// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlPlayApp/Tools/IToolWidget.h>

namespace tl
{
    namespace play
    {
        class App;

        //! View tool.
        //!
        //! \todo Add a background grid.
        class ViewTool : public IToolWidget
        {
            DTK_NON_COPYABLE(ViewTool);

        protected:
            void _init(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<App>&,
                const std::shared_ptr<IWidget>& parent);

            ViewTool();

        public:
            virtual ~ViewTool();

            static std::shared_ptr<ViewTool> create(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<App>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

        private:
            DTK_PRIVATE();
        };
    }
}
