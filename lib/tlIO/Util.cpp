// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlIO/Util.h>

#include <tlIO/IOSystem.h>

#include <tlCore/Context.h>

namespace tl
{
    namespace io
    {
        void init(const std::shared_ptr<system::Context>& context)
        {
            if (!context->getSystem<System>())
            {
                context->addSystem(System::create(context));
            }
        }
    }
}