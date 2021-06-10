// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrCoreTest/MemoryTest.h>

#include <tlrCore/Assert.h>
#include <tlrCore/Memory.h>

#include <sstream>

using namespace tlr::memory;

namespace tlr
{
    namespace CoreTest
    {
        MemoryTest::MemoryTest() :
            ITest("CoreTest::MemoryTest")
        {}

        std::shared_ptr<MemoryTest> MemoryTest::create()
        {
            return std::shared_ptr<MemoryTest>(new MemoryTest);
        }

        void MemoryTest::run()
        {
            {
                std::stringstream ss;
                ss << "Current endian: " << getEndian();
                _print(ss.str());
            }
            {
                std::stringstream ss;
                ss << "Opposite endian: " << opposite(getEndian());
                _print(ss.str());
            }
        }
    }
}
