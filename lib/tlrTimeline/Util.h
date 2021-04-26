// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#pragma once

#include <opentimelineio/version.h>

#include <opentime/rationalTime.h>
#include <opentime/timeRange.h>

#include <iostream>

namespace tlr
{
    namespace otime = opentime::OPENTIME_VERSION;
    namespace otio = opentimelineio::OPENTIMELINEIO_VERSION;

    std::ostream& operator << (std::ostream&, const otime::RationalTime&);
    std::ostream& operator << (std::ostream&, const otime::TimeRange&);
}
