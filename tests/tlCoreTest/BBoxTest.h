// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTestLib/ITest.h>

namespace tl
{
    namespace core_tests
    {
        class BBoxTest : public tests::ITest
        {
        protected:
            BBoxTest(const std::shared_ptr<system::Context>&);

        public:
            static std::shared_ptr<BBoxTest> create(const std::shared_ptr<system::Context>&);

            void run() override;

        private:
            void _ctors();
            void _components();
            void _dimensions();
            void _intersections();
            void _expand();
            void _margin();
            void _operators();
            void _serialize();
        };
    }
}
