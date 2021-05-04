// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrCore/Assert.h>

#include <iostream>

#include <stdlib.h>

namespace tlr
{
    namespace core
    {
        void _assert(const char* file, int line)
        {
            std::cout << "ASSERT: " << file << ":" << line << std::endl;
            abort();
        }
    }
}