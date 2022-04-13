// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <memory>

namespace tl
{
    namespace system
    {
        class Context;
    }

    //! Blackmagic Design DeckLink support.
    namespace dl
    {
        //! Initialize the library.
        void init(const std::shared_ptr<system::Context>&);

        //! Shutdown the library.
        void shutdown();
    }
}
