// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <memory>
#include <string>

namespace dtk
{
    class Context;
}

namespace tl
{
    //! Timeline user interface
    namespace timelineui
    {
        //! Initialize the library.
        void init(const std::shared_ptr<dtk::Context>&);
    }
}
