// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlUSD/Util.h>

#include <tlUSD/USD.h>

#include <tlIO/IOSystem.h>

#include <tlCore/Context.h>

namespace tl
{
    namespace usd
    {
        void init(const std::shared_ptr<system::Context>& context)
        {
            auto logSystem = context->getSystem<log::System>();
            auto ioSystem = context->getSystem<io::System>();
            ioSystem->addPlugin(Plugin::create(logSystem));
        }
    }
}

