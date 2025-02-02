// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <memory>

namespace dtk
{
    class Context;
}

namespace tl
{
    //! Player application support
    namespace play
    {
        //! Initialize the library.
        void init(const std::shared_ptr<dtk::Context>&);
    }
}
