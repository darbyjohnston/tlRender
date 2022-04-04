// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlPlayApp/ToolWidget.h>

namespace tl
{
    namespace system
    {
        class Context;
    }

    namespace play
    {
        //! Messages tool.
        class MessagesTool : public ToolWidget
        {
            Q_OBJECT

        public:
            MessagesTool(
                const std::shared_ptr<system::Context>&,
                QWidget* parent = nullptr);

            ~MessagesTool() override;

        private:
            TLRENDER_PRIVATE();
        };
    }
}
