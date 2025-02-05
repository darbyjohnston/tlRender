// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUI/IWidget.h>

namespace tl
{
    namespace ui
    {
        //! Base class for popups.
        class IPopup : public IWidget
        {
            DTK_NON_COPYABLE(IPopup);

        protected:
            void _init(
                const std::string& objectName,
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            IPopup();

        public:
            virtual ~IPopup() = 0;

            //! Close the popup.
            virtual void close() = 0;

            void keyPressEvent(KeyEvent&) override;
            void keyReleaseEvent(KeyEvent&) override;
        };
    }
}
