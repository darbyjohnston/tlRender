// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlPlay/ToolWidget.h>

namespace tl
{
    namespace play
    {
        class ColorModel;

        //! Color tool.
        class ColorTool : public ToolWidget
        {
            Q_OBJECT

        public:
            ColorTool(
                const std::shared_ptr<ColorModel>& colorModel,
                QWidget* parent = nullptr);

            ~ColorTool() override;

        private:
            void _widgetUpdate();

            TLRENDER_PRIVATE();
        };
    }
}
