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

        //! Color tool.
        class ColorTool : public IToolWidget
        {
            TLRENDER_NON_COPYABLE(ColorTool);

        protected:
            void _init(
                const std::shared_ptr<App>&,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent);

            ColorTool();

        public:
            virtual ~ColorTool();

            static std::shared_ptr<ColorTool> create(
                const std::shared_ptr<App>&,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

        private:
            TLRENDER_PRIVATE();
        };
    }
}
