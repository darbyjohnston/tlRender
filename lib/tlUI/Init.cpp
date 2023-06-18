// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlUI/Init.h>

#include <tlIO/Init.h>

#include <tlCore/Context.h>

#if defined(TLRENDER_NFD)
#include <nfd.hpp>
#endif // TLRENDER_NFD

namespace tl
{
    namespace ui
    {
        void init(const std::shared_ptr<system::Context>& context)
        {
            tl::io::init(context);
            if (!context->getSystem<System>())
            {
                context->addSystem(System::create(context));
            }
        }

        void System::_init(const std::shared_ptr<system::Context>& context)
        {
            ISystem::_init("tl::ui::System", context);

#if defined(TLRENDER_NFD)
            NFD::Init();
#endif // TLRENDER_NFD
        }

        System::System()
        {}

        System::~System()
        {
#if defined(TLRENDER_NFD)
            NFD::Quit();
#endif // TLRENDER_NFD
        }

        std::shared_ptr<System> System::create(const std::shared_ptr<system::Context>& context)
        {
            auto out = std::shared_ptr<System>(new System);
            out->_init(context);
            return out;
        }
    }
}
