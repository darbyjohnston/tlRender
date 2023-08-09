// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlUI/IPopup.h>

namespace tl
{
    namespace ui
    {
        void IPopup::_init(
            const std::string& objectName,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init(objectName, context, parent);
        }

        IPopup::IPopup()
        {}

        IPopup::~IPopup()
        {}
    }
}
