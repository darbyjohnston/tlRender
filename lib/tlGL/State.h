// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/Util.h>

#include <memory>

namespace tl
{
    namespace gl
    {
        //! Set whether an OpenGL capability is enabled (e.g., glEnable()),
        //! and restore it to the previous value when finished.
        class SetAndRestore
        {
        public:
            SetAndRestore(unsigned int, bool);
            
            ~SetAndRestore();

        private:
            TLRENDER_PRIVATE();
        };
    }
}
