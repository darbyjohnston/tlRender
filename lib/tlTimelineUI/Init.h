// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#pragma once

#include <memory>
#include <string>

namespace ftk
{
    class Context;
}

namespace tl
{
    //! Timeline user interface
    namespace timelineui
    {
        //! Initialize the library.
        void init(const std::shared_ptr<ftk::Context>&);
    }
}
