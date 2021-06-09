// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#pragma once

#include <tlrCore/Util.h>

#include <memory>

namespace tlr
{
    namespace Test
    {
        class ITest : public std::enable_shared_from_this<ITest>
        {
            TLR_NON_COPYABLE(ITest);

        protected:
            ITest(const std::string& name);

        public:
            virtual ~ITest() = 0;

            const std::string& getName() const;

            virtual void run() = 0;

        protected:
            void _print(const std::string&);
            void _printError(const std::string&);

        private:
            std::string _name;
        };
    }
}
