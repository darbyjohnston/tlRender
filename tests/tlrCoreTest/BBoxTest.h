// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#pragma once

#include <tlrTestLib/ITest.h>

namespace tlr
{
    namespace CoreTest
    {
        class BBoxTest : public Test::ITest
        {
        protected:
            BBoxTest();

        public:
            static std::shared_ptr<BBoxTest> create();

            void run() override;

        private:
            void _ctors();
            void _components();
            void _dimensions();
            void _intersections();
            void _expand();
            void _margin();
            void _operators();
        };
    }
}
