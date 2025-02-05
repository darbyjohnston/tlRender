// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUI/Action.h>
#include <tlUI/IWidget.h>

#include <tlTimeline/Player.h>

namespace tl
{
    namespace play_app
    {
        class App;

        //! Compare tool bar.
        class CompareToolBar : public ui::IWidget
        {
            DTK_NON_COPYABLE(CompareToolBar);

        protected:
            void _init(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<App>&,
                const std::map<std::string, std::shared_ptr<ui::Action> >&,
                const std::shared_ptr<IWidget>& parent);

            CompareToolBar();

        public:
            ~CompareToolBar();

            static std::shared_ptr<CompareToolBar> create(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<App>&,
                const std::map<std::string, std::shared_ptr<ui::Action> >&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            void setGeometry(const dtk::Box2I&) override;
            void sizeHintEvent(const ui::SizeHintEvent&) override;

        private:
            void _compareUpdate(const timeline::CompareOptions&);

            DTK_PRIVATE();
        };
    }
}
