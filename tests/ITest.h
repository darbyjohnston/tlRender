// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#pragma once

#include <tlrCore/Util.h>

#include <memory>

namespace tlr
{
    class ITest : public std::enable_shared_from_this<ITest>
    {
        TLR_NON_COPYABLE(ITest);

    protected:
        ITest();

    public:
        virtual ~ITest() = 0;

        virtual void run() = 0;
    };
}
