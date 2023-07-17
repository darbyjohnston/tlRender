// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlPlayGLApp/IToolWidget.h>

namespace tl
{
    namespace play_gl
    {
        class App;

        //! System log tool widget.
        class SystemLogToolWidget : public IToolWidget
        {
            TLRENDER_NON_COPYABLE(SystemLogToolWidget);

        protected:
            void _init(
                const std::shared_ptr<App>&,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent);

            SystemLogToolWidget();

        public:
            virtual ~SystemLogToolWidget();

            static std::shared_ptr<SystemLogToolWidget> create(
                const std::shared_ptr<App>&,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

        private:
            TLRENDER_PRIVATE();
        };
    }
}
