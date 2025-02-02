// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlTimelineTest/IRenderTest.h>

#include <tlTimeline/IRender.h>

using namespace tl::timeline;

namespace tl
{
    namespace timeline_tests
    {
        IRenderTest::IRenderTest(const std::shared_ptr<dtk::Context>& context) :
            ITest(context, "timeline_tests::IRenderTest")
        {}

        std::shared_ptr<IRenderTest> IRenderTest::create(const std::shared_ptr<dtk::Context>& context)
        {
            return std::shared_ptr<IRenderTest>(new IRenderTest(context));
        }

        void IRenderTest::run()
        {
            _enums();
        }

        void IRenderTest::_enums()
        {
            _enum<InputVideoLevels>("InputVideoLevels", getInputVideoLevelsEnums);
            _enum<Channels>("Channels", getChannelsEnums);
            _enum<AlphaBlend>("AlphaBlend", getAlphaBlendEnums);
        }
    }
}
