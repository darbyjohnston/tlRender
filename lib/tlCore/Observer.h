// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020 Darby Johnston
// All rights reserved.

#pragma once

namespace tl
{
    //! Observer pattern
    namespace observer
    {
        //! Observer callback options.
        enum class CallbackAction
        {
            Trigger,
            Suppress
        };
    }
}
