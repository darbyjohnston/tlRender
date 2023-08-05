// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlPlayGLApp/Tools.h>

#include <tlUI/IWidget.h>

namespace tl
{
    namespace play_gl
    {
        class App;

        //! Base class for tool widgets.
        class IToolWidget : public ui::IWidget
        {
            TLRENDER_NON_COPYABLE(IToolWidget);

        protected:
            void _init(
                Tool,
                const std::string& name,
                const std::shared_ptr<App>&,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent);

            IToolWidget();

        public:
            virtual ~IToolWidget() = 0;

            void setGeometry(const math::Box2i&) override;
            void sizeHintEvent(const ui::SizeHintEvent&) override;

        protected:
            void _setWidget(const std::shared_ptr<ui::IWidget>&);

            std::weak_ptr<App> _app;

        private:
            TLRENDER_PRIVATE();
        };
    }
}
