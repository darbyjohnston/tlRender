// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlPlay/ToolWidget.h>

namespace tl
{
    namespace avio
    {
        struct Info;
    }

    namespace play
    {
        //! Information tool.
        class InfoTool : public ToolWidget
        {
            Q_OBJECT

        public:
            InfoTool(QWidget* parent = nullptr);

            ~InfoTool() override;

            void setInfo(const avio::Info&);

        private:
            TLRENDER_PRIVATE();
        };
    }
}
