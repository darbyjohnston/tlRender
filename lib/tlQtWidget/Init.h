// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlQt/Init.h>

namespace tl
{
    //! Qt QWidget support
    namespace qtwidget
    {
        //! Initialize the library. This needs to be called before the Qt
        //! application is created.
        void init(
            const std::shared_ptr<dtk::Context>&,
            qt::DefaultSurfaceFormat);

        //! Initialize the fonts. This needs to be called after the Qt
        //! application is created.
        void initFonts(const std::shared_ptr<dtk::Context>& context);

        //! Qt QWidget system.
        class System : public system::ISystem
        {
            DTK_NON_COPYABLE(System);

        protected:
            System(const std::shared_ptr<dtk::Context>&);

        public:
            virtual ~System();

            //! Create a new system.
            static std::shared_ptr<System> create(const std::shared_ptr<dtk::Context>&);
        };
    }
}
