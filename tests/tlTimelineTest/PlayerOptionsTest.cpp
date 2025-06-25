// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlTimelineTest/PlayerOptionsTest.h>

#include <tlTimeline/PlayerOptions.h>

#include <feather-tk/core/Assert.h>
#include <feather-tk/core/String.h>

using namespace tl::timeline;

namespace tl
{
    namespace timeline_tests
    {
        PlayerOptionsTest::PlayerOptionsTest(const std::shared_ptr<feather_tk::Context>& context) :
            ITest(context, "timeline_tests::PlayerOptionsTest")
        {}

        std::shared_ptr<PlayerOptionsTest> PlayerOptionsTest::create(const std::shared_ptr<feather_tk::Context>& context)
        {
            return std::shared_ptr<PlayerOptionsTest>(new PlayerOptionsTest(context));
        }

        void PlayerOptionsTest::run()
        {
            {
                PlayerCacheOptions v;
                v.readBehind = 0.F;
                FEATHER_TK_ASSERT(v == v);
                FEATHER_TK_ASSERT(v != PlayerCacheOptions());
            }
        }
    }
}
