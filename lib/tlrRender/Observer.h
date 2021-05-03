// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020 Darby Johnston
// All rights reserved.

#pragma once

namespace tlr
{
    //! Observer pattern.
    namespace Observer
    {
        //! Observer callback options.
        enum class CallbackAction
        {
            Trigger,
            Suppress
        };
    }
}
