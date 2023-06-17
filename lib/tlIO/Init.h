// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <memory>

namespace tl
{
    namespace system
    {
        class Context;
    }

    //! I/O library.
    namespace io
    {
        //! Initialize the library.
        void init(const std::shared_ptr<system::Context>&);
    }
}