// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlAppPlay/ToolWidget.h>

namespace tl
{
    namespace core
    {
        namespace system
        {
            class Context;
        }
    }

    namespace app
    {
        namespace play
        {
            //! Messages tool.
            class MessagesTool : public ToolWidget
            {
                Q_OBJECT

            public:
                MessagesTool(
                    const std::shared_ptr<core::system::Context>&,
                    QWidget* parent = nullptr);

                ~MessagesTool() override;

            private:
                TLRENDER_PRIVATE();
            };
        }
    }
}
