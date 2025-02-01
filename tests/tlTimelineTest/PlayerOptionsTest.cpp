// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlTimelineTest/PlayerOptionsTest.h>

#include <tlTimeline/PlayerOptions.h>

#include <dtk/core/Assert.h>
#include <dtk/core/String.h>

using namespace tl::timeline;

namespace tl
{
    namespace timeline_tests
    {
        PlayerOptionsTest::PlayerOptionsTest(const std::shared_ptr<system::Context>& context) :
            ITest("timeline_tests::PlayerOptionsTest", context)
        {}

        std::shared_ptr<PlayerOptionsTest> PlayerOptionsTest::create(const std::shared_ptr<system::Context>& context)
        {
            return std::shared_ptr<PlayerOptionsTest>(new PlayerOptionsTest(context));
        }

        void PlayerOptionsTest::run()
        {
            {
                PlayerCacheOptions v;
                v.readAhead = otime::RationalTime(10.0, 1.0);
                DTK_ASSERT(v == v);
                DTK_ASSERT(v != PlayerCacheOptions());
            }
        }
    }
}
