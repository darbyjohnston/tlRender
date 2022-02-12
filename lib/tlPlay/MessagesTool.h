// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlPlay/ToolWidget.h>

namespace tl
{
    namespace core
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
                const std::shared_ptr<core::Context>&,
                QWidget* parent = nullptr);

        private:
            TLRENDER_PRIVATE();
        };
    }
}
