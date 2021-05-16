// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#pragma once

#include <tlrCore/Util.h>

#include <chrono>
#include <vector>

namespace tlr
{
    //! Time.
    namespace time
    {
        //! Sleep for the given time.
        void sleep(const std::chrono::microseconds&);
    }
}
