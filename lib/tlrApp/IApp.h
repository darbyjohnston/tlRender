// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#pragma once

#include <tlrCore/Util.h>

#include <memory>
#include <string>
#include <vector>

namespace tlr
{
    //! Application
    namespace app
    {
        //! Application Interface
        class IApp : public std::enable_shared_from_this<IApp>
        {
            TLR_NON_COPYABLE(IApp);

        protected:
            void _init(int argc, char* argv[]);
            IApp();

        public:
            virtual ~IApp() = 0;

        protected:
            std::vector<std::string> _args;
        };
    }
}
