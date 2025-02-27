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

        //! Devices tool.
        class DevicesTool : public IToolWidget
        {
            DTK_NON_COPYABLE(DevicesTool);

        protected:
            void _init(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<App>&,
                const std::shared_ptr<IWidget>& parent);

            DevicesTool();

        public:
            virtual ~DevicesTool();

            static std::shared_ptr<DevicesTool> create(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<App>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

        private:
            DTK_PRIVATE();
        };
    }
}
