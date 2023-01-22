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

    //! Qt Quick support.
    namespace qtquick
    {
        //! Initialize the library. This needs to be called before the Qt
        //! application is created.
        void init(const std::shared_ptr<system::Context>&);

        //! Get the context singleton.
        //!
        //! \todo What's a better way to get the context to QML objects?
        const std::weak_ptr<system::Context>& context();
    }
}
