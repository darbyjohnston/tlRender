// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlPlayApp/Tools.h>

#include <tlUI/IWidget.h>

namespace tl
{
    namespace play_app
    {
        class App;
        class MainWindow;

        //! Tools widget.
        class ToolsWidget : public ui::IWidget
        {
            TLRENDER_NON_COPYABLE(ToolsWidget);

        protected:
            void _init(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<App>&,
                const std::shared_ptr<MainWindow>&,
                const std::shared_ptr<IWidget>& parent);

            ToolsWidget();

        public:
            virtual ~ToolsWidget();

            //! Create a new widget.
            static std::shared_ptr<ToolsWidget> create(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<App>&,
                const std::shared_ptr<MainWindow>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            void setGeometry(const dtk::Box2I&) override;
            void sizeHintEvent(const ui::SizeHintEvent&) override;

        private:
            TLRENDER_PRIVATE();
        };
    }
}
