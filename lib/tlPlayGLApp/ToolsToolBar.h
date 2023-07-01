// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlUI/IWidget.h>

#include <tlTimeline/Player.h>

namespace tl
{
    namespace play_gl
    {
        class App;

        //! Tools tool bar.
        class ToolsToolBar : public ui::IWidget
        {
            TLRENDER_NON_COPYABLE(ToolsToolBar);

        protected:
            void _init(
                const std::shared_ptr<App>&,
                const std::shared_ptr<system::Context>&);

            ToolsToolBar();

        public:
            ~ToolsToolBar();

            static std::shared_ptr<ToolsToolBar> create(
                const std::shared_ptr<App>&,
                const std::shared_ptr<system::Context>&);

            void setGeometry(const math::BBox2i&) override;
            void sizeHintEvent(const ui::SizeHintEvent&) override;

        private:
            TLRENDER_PRIVATE();
        };
    }
}
