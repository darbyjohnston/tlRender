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

    //! Qt support.
    namespace qt
    {
        //! Initialize the library. This needs to be called before the Qt
        //! application is created.
        void init(const std::shared_ptr<system::Context>&);
    }
}
