// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#pragma once

#include <tlrTestLib/ITest.h>

namespace tlr
{
    namespace CoreTest
    {
        class MapObserverTest : public Test::ITest
        {
        protected:
            MapObserverTest();

        public:
            static std::shared_ptr<MapObserverTest> create();

            void run() override;
        };
    }
}
