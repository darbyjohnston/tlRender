// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlTimeline/Transition.h>

#include <tlCore/Time.h>

#include <feather-tk/core/Error.h>
#include <feather-tk/core/String.h>

#include <opentimelineio/transition.h>

#include <array>
#include <sstream>

namespace tl
{
    namespace timeline
    {
        FEATHER_TK_ENUM_IMPL(
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