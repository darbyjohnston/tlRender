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

        //! Base class for tool widgets.
        class IToolWidget : public ui::IWidget
        {
            DTK_NON_COPYABLE(IToolWidget);

        protected:
            void _init(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<App>&,
                Tool,
                const std::string& objectName,
                const std::shared_ptr<IWidget>& parent);

            IToolWidget();

        public:
            virtual ~IToolWidget() = 0;

            void setGeometry(const dtk::Box2I&) override;
            void sizeHintEvent(const ui::SizeHintEvent&) override;

        protected:
            void _setWidget(const std::shared_ptr<ui::IWidget>&);

            std::weak_ptr<App> _app;

        private:
            DTK_PRIVATE();
        };
    }
}
