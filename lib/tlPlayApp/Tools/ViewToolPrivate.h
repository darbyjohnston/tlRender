// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlPlayApp/Tools/ViewTool.h>

#include <tlTimeline/BackgroundOptions.h>

namespace tl
{
    namespace play
    {
        class BackgroundWidget : public dtk::IWidget
        {
            DTK_NON_COPYABLE(BackgroundWidget);

        protected:
            void _init(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<App>&,
                const std::shared_ptr<IWidget>& parent);

            BackgroundWidget();

        public:
            virtual ~BackgroundWidget();

            static std::shared_ptr<BackgroundWidget> create(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<App>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            void setGeometry(const dtk::Box2I&) override;
            void sizeHintEvent(const dtk::SizeHintEvent&) override;

        private:
            void _optionsUpdate(const timeline::BackgroundOptions&);

            DTK_PRIVATE();
        };
    }
}
