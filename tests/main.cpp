// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include "TimelineTest.h"

#include <vector>

int main(int argc, char* argv[])
{
    std::vector<std::shared_ptr<tlr::ITest> > tests;
    tests.push_back(tlr::TimelineTest::create());

    for (const auto& i : tests)
    {
        i->run();
    }

    return 0;
}
