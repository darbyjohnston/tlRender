// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlGlad/gl.h>

#include <memory>

namespace tl
{
    namespace core
    {
        namespace system
        {
            class Context;
        }
    }

    namespace qt
    {
        //! Qt Quick functionality.
        namespace quick
        {
            //! Initialize the library. This needs to be called before the Qt application is instantiated.
            void init();

            //! Set the context singleton.
            //!
            //! \todo What's a better way to get the contet to QML objects?
            void setContext(const std::shared_ptr<core::system::Context>&);

            //! Get the context singleton.
            const std::weak_ptr<core::system::Context>& context();
        }
    }
}
