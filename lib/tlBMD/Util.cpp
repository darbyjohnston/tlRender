// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlBMD/Util.h>

#include <tlBMD/DeviceInfo.h>

#include <tlCore/Context.h>

#include "platform.h"

#include <stdexcept>

namespace tl
{
    namespace bmd
    {
        void init(const std::shared_ptr<system::Context>& context)
        {
#if defined(_WIN32)
            HRESULT result = CoInitialize(NULL);
            if (FAILED(result))
            {
                throw std::runtime_error("COM initialization failed");
            }
#endif // _WIN32

            if (!context->getSystem<DeviceInfoSystem>())
            {
                context->addSystem(DeviceInfoSystem::create(context));
            }
        }

        void shutdown()
        {
#if defined(_WIN32)
            CoUninitialize();
#endif // _WIN32
        }
    }
}
