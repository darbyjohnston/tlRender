// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlPlayApp/ToolWidget.h>

#include <tlQt/MetaTypes.h>

namespace tl
{
    namespace play
    {
        class App;

        //! Device tool.
        class DeviceTool : public ToolWidget
        {
            Q_OBJECT

        public:
            DeviceTool(
                App*,
                QWidget* parent = nullptr);

            ~DeviceTool() override;

        private Q_SLOTS:
            void _deviceCallback(int);
            void _displayModeCallback(int);
            void _pixelTypeCallback(int);

        private:
            TLRENDER_PRIVATE();
        };
    }
}
