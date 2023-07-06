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

        //! Compare tool bar.
        class CompareToolBar : public ui::IWidget
        {
            TLRENDER_NON_COPYABLE(CompareToolBar);

        protected:
            void _init(
                const std::shared_ptr<App>&,
                const std::shared_ptr<system::Context>&);

            CompareToolBar();

        public:
            ~CompareToolBar();

            static std::shared_ptr<CompareToolBar> create(
                const std::shared_ptr<App>&,
                const std::shared_ptr<system::Context>&);

            void setGeometry(const math::BBox2i&) override;
            void sizeHintEvent(const ui::SizeHintEvent&) override;

        private:
            void _compareUpdate(const timeline::CompareOptions&);

            TLRENDER_PRIVATE();
        };
    }
}
