// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlCoreTest/OSTest.h>

#include <tlCore/OS.h>

#include <dtk/core/String.h>

#include <sstream>

using namespace tl::os;

namespace tl
{
    namespace core_tests
    {
        OSTest::OSTest(const std::shared_ptr<system::Context>& context) :
            ITest("core_tests::OSTest", context)
        {}

        std::shared_ptr<OSTest> OSTest::create(const std::shared_ptr<system::Context>& context)
        {
            return std::shared_ptr<OSTest>(new OSTest(context));
        }

        void OSTest::run()
        {
            {
                const auto si = getSystemInfo();
                std::stringstream ss;
                ss << "System name: " << si.name;
                _print(ss.str());
            }
            {
                std::stringstream ss;
                ss << "Environment variable list separator: " << envListSeparator;
                _print(ss.str());
            }
            {
                const std::string env = "OSTEST";
                const std::string value = "1";
                DTK_ASSERT(setEnv(env, value));
                std::string value2;
                DTK_ASSERT(getEnv(env, value2));
                DTK_ASSERT(value == value2);
                DTK_ASSERT(delEnv(env));
            }
            {
                const std::string env = "OSTEST";
                std::string value;
                DTK_ASSERT(!getEnv(env, value));
            }
            {
                const std::string env = "OSTEST";
                const int value = 1;
                std::stringstream ss;
                ss << value;
                DTK_ASSERT(setEnv(env, ss.str()));
                int value2 = 0;
                DTK_ASSERT(getEnv(env, value2));
                DTK_ASSERT(value == value2);
                DTK_ASSERT(delEnv(env));
            }
            {
                const std::string env = "OSTEST";
                int value = 0;
                DTK_ASSERT(!getEnv(env, value));
            }
            {
                const std::string env = "OSTEST";
                const std::vector<std::string> value = { "a", "b", "c" };
                DTK_ASSERT(setEnv(env, dtk::join(value, envListSeparator)));
                std::vector<std::string> value2;
                DTK_ASSERT(getEnv(env, value2));
                DTK_ASSERT(value == value2);
                DTK_ASSERT(delEnv(env));
            }
            {
                const std::string env = "OSTEST";
                std::vector<std::string> value;
                DTK_ASSERT(!getEnv(env, value));
            }
        }
    }
}
