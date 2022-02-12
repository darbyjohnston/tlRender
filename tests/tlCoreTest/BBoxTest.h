// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTestLib/ITest.h>

namespace tl
{
    namespace CoreTest
    {
        class BBoxTest : public Test::ITest
        {
        protected:
            BBoxTest(const std::shared_ptr<core::Context>&);

        public:
            static std::shared_ptr<BBoxTest> create(const std::shared_ptr<core::Context>&);

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
