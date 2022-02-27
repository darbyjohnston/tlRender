// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlAppPlay/ToolWidget.h>

namespace tl
{
    namespace core
    {
        class Context;
    }

    namespace app
    {
        namespace play
        {
            //! System log tool.
            class SystemLogTool : public ToolWidget
            {
                Q_OBJECT

            public:
                SystemLogTool(
                    const std::shared_ptr<core::Context>&,
                    QWidget* parent = nullptr);

                ~SystemLogTool() override;

            private:
                TLRENDER_PRIVATE();
            };
        }
    }
}
