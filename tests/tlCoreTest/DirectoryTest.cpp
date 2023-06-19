// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlCoreTest/DirectoryTest.h>

#include <tlCore/Directory.h>

using namespace tl::file;

namespace tl
{
    namespace core_tests
    {
        DirectoryTest::DirectoryTest(const std::shared_ptr<system::Context>& context) :
            ITest("core_tests::DirectoryTest", context)
        {}

        std::shared_ptr<DirectoryTest> DirectoryTest::create(const std::shared_ptr<system::Context>& context)
        {
            return std::shared_ptr<DirectoryTest>(new DirectoryTest(context));
        }

        void DirectoryTest::run()
        {
            _tests();
        }

        void DirectoryTest::_tests()
        {
            for (const auto& i : list("."))
            {
                _print(i.getPath().get());
            }
        }
    }
}
