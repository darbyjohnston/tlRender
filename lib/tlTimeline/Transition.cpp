// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlTimeline/Transition.h>

#include <tlCore/Time.h>

#include <dtk/core/Error.h>
#include <dtk/core/String.h>

#include <opentimelineio/transition.h>

#include <array>
#include <sstream>

namespace tl
{
    namespace timeline
    {
        TLRENDER_ENUM_IMPL(
            Transition,
            "None",
            "Dissolve");
        TLRENDER_ENUM_SERIALIZE_IMPL(Transition);

        Transition toTransition(const std::string& value)
        {
            Transition out = Transition::None;
            if (otio::Transition::Type::SMPTE_Dissolve == value)
            {
                out = Transition::Dissolve;
            }
            return out;
        }
    }
}