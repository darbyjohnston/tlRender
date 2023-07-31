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

        //! System log tool.
        //! 
        //! \todo Add an option to automatically scroll to the bottom when
        //! new log items are received.
        class SystemLogTool : public IToolWidget
        {
            TLRENDER_NON_COPYABLE(SystemLogTool);

        protected:
            void _init(
                const std::shared_ptr<App>&,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent);

            SystemLogTool();

        public:
            virtual ~SystemLogTool();

            static std::shared_ptr<SystemLogTool> create(
                const std::shared_ptr<App>&,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

        private:
            TLRENDER_PRIVATE();
        };
    }
}
