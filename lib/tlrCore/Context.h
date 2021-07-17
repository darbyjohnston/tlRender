// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#pragma once

#include <tlrCore/AVIO.h>

namespace tlr
{
    namespace core
    {
        //! Context.
        class Context
        {
            TLR_NON_COPYABLE(Context);

        protected:
            void _init();
            Context();

        public:
            ~Context();

            //! Create a new context.
            static std::shared_ptr<Context> create();

            //! Get the AV I/O system.
            std::shared_ptr<avio::System> getAVIOSystem() const;

        private:
            TLR_PRIVATE();
        };
    }
}
