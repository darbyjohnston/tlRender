// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUI/Action.h>
#include <tlUI/IWidget.h>

#include <tlTimeline/Player.h>

namespace tl
{
    namespace play_gl
    {
        class App;

        //! Window tool bar.
        class WindowToolBar : public ui::IWidget
        {
            TLRENDER_NON_COPYABLE(WindowToolBar);

        protected:
            void _init(
                const std::map<std::string, std::shared_ptr<ui::Action> >&,
                const std::shared_ptr<App>&,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent);

            WindowToolBar();

        public:
            ~WindowToolBar();

            static std::shared_ptr<WindowToolBar> create(
                const std::map<std::string, std::shared_ptr<ui::Action> >&,
                const std::shared_ptr<App>&,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            void setGeometry(const math::BBox2i&) override;
            void sizeHintEvent(const ui::SizeHintEvent&) override;

        private:
            TLRENDER_PRIVATE();
        };
    }
}
