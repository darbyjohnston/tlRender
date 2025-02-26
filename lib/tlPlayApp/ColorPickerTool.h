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

        //! Color picker tool.
        class ColorPickerTool : public IToolWidget
        {
            DTK_NON_COPYABLE(ColorPickerTool);

        protected:
            void _init(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<App>&,
                const std::shared_ptr<IWidget>& parent);

            ColorPickerTool();

        public:
            virtual ~ColorPickerTool();

            static std::shared_ptr<ColorPickerTool> create(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<App>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

        private:
            void _widgetUpdate();

            DTK_PRIVATE();
        };
    }
}
