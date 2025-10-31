// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

namespace tl
{
    namespace time
    {
        inline bool isValid(const OTIO_NS::RationalTime& value)
        {
            return !value.is_invalid_time();
        }

        inline bool isValid(const OTIO_NS::TimeRange& value)
        {
            return
                !value.start_time().is_invalid_time() &&
                !value.duration().is_invalid_time();
        }

        constexpr bool compareExact(const OTIO_NS::TimeRange& a, const OTIO_NS::TimeRange& b)
        {
            return
                a.start_time().strictly_equal(b.start_time()) &&
                a.duration().strictly_equal(b.duration());
        }
    }
}

