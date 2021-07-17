// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrCore/Context.h>

namespace tlr
{
    namespace core
    {
        struct Context::Private
        {
            std::shared_ptr<avio::System> avioSystem;
        };

        void Context::_init()
        {
            TLR_PRIVATE_P();

            p.avioSystem = avio::System::create();
        }

        Context::Context() :
            _p(new Private)
        {}

        Context::~Context()
        {}

        std::shared_ptr<Context> Context::create()
        {
            auto out = std::shared_ptr<Context>(new Context);
            out->_init();
            return out;
        }

        std::shared_ptr<avio::System> Context::getAVIOSystem() const
        {
            return _p->avioSystem;
        }
    }
}