// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTestLib/ITest.h>

#include <tlTimeline/Timeline.h>

namespace tl
{
    namespace timeline_tests
    {
        class TimelineTest : public tests::ITest
        {
        protected:
            TimelineTest(const std::shared_ptr<system::Context>&);

        public:
            static std::shared_ptr<TimelineTest> create(const std::shared_ptr<system::Context>&);

            void run() override;

        private:
            void _enums();
            void _util();
            void _transitions();
            void _videoData();
            void _create();
            void _timeline();
            void _timeline(
                const std::string& fileName,
                const image::Info&,
                const std::shared_ptr<timeline::Timeline>&);
            void _imageSequence();
        };
    }
}
