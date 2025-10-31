// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <tlTimeline/Transition.h>

#include <tlCore/Time.h>

#include <ftk/Core/Error.h>
#include <ftk/Core/String.h>

#include <opentimelineio/transition.h>

#include <array>
#include <sstream>

namespace tl
{
    namespace timeline
    {
        FTK_ENUM_IMPL(
            Transition,
            "None",
            "Dissolve");

        Transition toTransition(const std::string& value)
        {
            Transition out = Transition::None;
            if (OTIO_NS::Transition::Type::SMPTE_Dissolve == value)
            {
                out = Transition::Dissolve;
            }
            return out;
        }
    }
}