// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#pragma once

#include <glad/gl.h>

#include <memory>

namespace tlr
{
    namespace core
    {
        class Context;
    }
    
    //! Qt Quick support.
    namespace quick
    {
        //! Initialize the library. This needs to be called before the Qt application is instantiated.
        void init();

        //! Set the context singleton.
        //!
        //! \todo What's a better way to get the contet to QML objects?
        void setContext(const std::shared_ptr<core::Context>&);
        
        //! Get the context singleton.
        const std::weak_ptr<core::Context>& context();
    }
}

