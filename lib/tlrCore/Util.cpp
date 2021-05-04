// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrCore/Util.h>

namespace tlr
{
    std::ostream& operator << (std::ostream& os, const opentime::RationalTime& value)
    {
        os << value.value() << "/" << value.rate();
        return os;
    }

    std::ostream& operator << (std::ostream& os, const opentime::TimeRange& value)
    {
        os << value.start_time() << "-" << value.duration();
        return os;
    }
}
