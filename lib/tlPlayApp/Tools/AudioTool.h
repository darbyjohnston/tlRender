// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlPlayApp/Tools/IToolWidget.h>

namespace tl
{
    namespace play
    {
        class App;

        //! Audio tool.
        class AudioTool : public IToolWidget
        {
            DTK_NON_COPYABLE(AudioTool);

        protected:
            void _init(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<App>&,
                const std::shared_ptr<IWidget>& parent);

            AudioTool();

        public:
            virtual ~AudioTool();

            static std::shared_ptr<AudioTool> create(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<App>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

        private:
            void _widgetUpdate();

            DTK_PRIVATE();
        };
    }
}
