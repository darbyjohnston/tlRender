// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#pragma once

#include "ITest.h"

namespace tlr
{
    class TimelineTest : public ITest
    {
    public:
        static std::shared_ptr<TimelineTest> create();

        void run() override;

    private:
        void _toRanges();
    };
}
