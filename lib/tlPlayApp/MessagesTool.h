// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlPlayApp/IToolWidget.h>

namespace tl
{
    namespace play_app
    {
        class App;

        //! Messages tool.
        class MessagesTool : public IToolWidget
        {
            DTK_NON_COPYABLE(MessagesTool);

        protected:
            void _init(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<App>&,
                const std::shared_ptr<IWidget>& parent);

            MessagesTool();

        public:
            virtual ~MessagesTool();

            static std::shared_ptr<MessagesTool> create(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<App>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

        private:
            DTK_PRIVATE();
        };
    }
}
